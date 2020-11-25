
#include "widgets/grid/PatternGrid.hpp"


#include <QFontDatabase>
#include <QPainter>

#include <QtDebug>

#include <algorithm>

// Optimization
// if drawing performance is an issue, implement partial redraws. Ie if only the
// cursor changes we just need to redraw where the cursor was and where it will be.
// Currently, the entire widget is redrawn on repaint, with the row text being cached
// in a QPixmap (mDisplay).

// A cell is a single character on the grid, columns are 1 or more cells. A
// cell with a space is used as spacing, and the adjacent column is selected if
// the user clicks on it.
//
// The first 4 cells of the grid are reserved for the row numbers, and are not selectable

// Track layout (. = space)
// 1 effect:  .NNN.II.EEE.          (12 chars)
// 2 effects: .NNN.II.EEE.EEE.      (16 chars)
// 3 effects: .NNN.II.EEE.EEE.EEE.  (20 chars)
//
// The minimum width of the grid is 4 + 12 * 4 = 52 chars
// the maximum width is 4 + 20 * 4 = 84 chars

namespace grid {

PatternGrid::PatternGrid(OrderModel &model, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mEffectsVisible{ 1 },
    mRepaintImage(true),
    mCursorRow(0),
    mCursorCol(0),
    mCursorPattern(0),
    mPatterns(1),
    mPatternSize(64),
    mPageStep(4),
    mWheel(0),
    mDisplayXpos(0),
    mRowHeight(0),
    mCharWidth(0),
    mVisibleRows(0)
{
    setFocusPolicy(Qt::StrongFocus);

    connect(&model, &OrderModel::trackChanged, this, &PatternGrid::setCursorTrack);


    // default colors
    mColorTable[COLOR_BG] = QColor(8, 24, 32);
    mColorTable[COLOR_BG_ROW] = QColor(20, 20, 80, 128);
    mColorTable[COLOR_CURSOR] = QColor(192, 192, 192, 128);
    mColorTable[COLOR_FG] = QColor(136, 192, 112);
    mColorTable[COLOR_FG_HIGHLIGHT] = QColor(224, 248, 208);
    mColorTable[COLOR_LINE] = QColor(64, 64, 64);

    mLineCells[0] = 0;
    mLineCells[1] = 4;

    // initialize layout
    uint8_t colIndex = 0;
    uint8_t colPos = 5;
    for (uint8_t i = 0; i != 4; ++i) {

        // '.' is a space
        // .C-5.00.V01.
        // 000011233455 - cell layout
        //  0   12 345  - columns


        // setup columns
        mColumns.push_back({ i, COLUMN_NOTE, colPos });
        colPos += 4;

        mColumns.push_back({ i, COLUMN_INSTRUMENT_HIGH, colPos });
        colPos += 1;

        mColumns.push_back({ i, COLUMN_INSTRUMENT_LOW, colPos });
        colPos += 2;

        mColumns.push_back({ i, COLUMN_EFFECT1_TYPE, colPos });
        colPos += 1;

        mColumns.push_back({ i, COLUMN_EFFECT1_ARG_HIGH, colPos });
        colPos += 1;

        mColumns.push_back({ i, COLUMN_EFFECT1_ARG_LOW, colPos });
        colPos += 3;

        // setup cell layout


        mCellLayout.push_back(colIndex); // the line cell before the note column
        mCellLayout.push_back(colIndex); // the next three cells are the note column
        mCellLayout.push_back(colIndex);
        mCellLayout.push_back(colIndex);
        ++colIndex;


        mCellLayout.push_back(colIndex); // space in between note and instrument high
        mCellLayout.push_back(colIndex); // instrument high
        ++colIndex;


        mCellLayout.push_back(colIndex); // instrument low
        ++colIndex;


        mCellLayout.push_back(colIndex); // space in between instrument low and effect1 type
        mCellLayout.push_back(colIndex); // effect1 type
        ++colIndex;


        mCellLayout.push_back(colIndex); // effect1 arg high
        ++colIndex;

        mCellLayout.push_back(colIndex); // effect1 arg low
        mCellLayout.push_back(colIndex); // space after effect1 arg low
        ++colIndex;

        mLineCells[static_cast<size_t>(i) + 2] = static_cast<unsigned>(mCellLayout.size() + 4);

    }
    // the layout changes whenever the user hides/shows extra effect columns


    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPixelSize(12);
    setFont(font);

    // initialize appearance settings for the first time
    appearanceChanged();

}

int PatternGrid::row() const {
    return mCursorRow;
}

// ================================================================= SLOTS ===

void PatternGrid::cursorLeft() {
    if (mCursorCol == 0) {
        mCursorCol = mColumns.size() - 1;
    } else {
        --mCursorCol;
    }
    update();
}

void PatternGrid::cursorRight() {
    if (++mCursorCol == mColumns.size()) {
        mCursorCol = 0;
    }
    update();
}

void PatternGrid::cursorUp() {
    setCursorRow(mCursorRow - 1);
    emit cursorRowChanged(mCursorRow);
}

void PatternGrid::cursorDown() {
    setCursorRow(mCursorRow + 1);
    emit cursorRowChanged(mCursorRow);
}

void PatternGrid::setCursorRow(int row) {

    if (row < 0) {
        row = -row;
        int patterns = (row / mPatternSize) + 1;
        mCursorRow = mPatternSize - (row % mPatternSize);
        mCursorPattern -= patterns;
        mCursorPattern %= mModel.rowCount();
        mRepaintImage = true;
        update();
    } else if (row >= mPatternSize) {
        int patterns = row / mPatternSize;
        mCursorRow = row % mPatternSize;
        mCursorPattern += patterns;
        mCursorPattern %= mModel.rowCount();
        mRepaintImage = true;
        update();
    } else {
        scroll(row - mCursorRow);
    }
}

void PatternGrid::setCursorTrack(int track) {
    if (track < 0 || track > 3) {
        return;
    }

    unsigned index = 0;
    for (auto col : mColumns) {
        if (col.type == COLUMN_NOTE && col.track == track) {
            mCursorCol = index;
            break;
        }
        ++index;
    }
    update();
}

// ================================================================ EVENTS ===

void PatternGrid::paintEvent(QPaintEvent *evt) {
    (void)evt;

    if (mRepaintImage) {
        mDisplay.fill(Qt::transparent);

        QPainter painter(&mDisplay);
        painter.setFont(font());

        paintRows(painter, 0, mVisibleRows);

        mRepaintImage = false;
    }

    auto const h = height();
    auto const w = width();
    unsigned const center = mVisibleRows / 2 * mRowHeight;

    QPainter painter(this);


    // background

    painter.fillRect(0, 0, w, h, mColorTable[COLOR_BG]);

    // highlights
    // TODO

    // cursor row
    painter.setPen(mColorTable[COLOR_BG_ROW]);
    painter.drawLine(0, center, w, center);
    painter.fillRect(0, center, w, mRowHeight, mColorTable[COLOR_BG_ROW]);
    painter.drawLine(0, center + mRowHeight - 1, w, center + mRowHeight - 1);


    // the grid is centered so translate everything else by the x offset
    if (mDisplayXpos > 0) {
        painter.translate(QPoint(mDisplayXpos, 0));
    }

    // selection
    // TODO

    // cursor
    auto &column = mColumns[mCursorCol];
    {
        // the cursor has a 1 pixel border around the width and height of a character
        // this way the cursor outline is not drawn under the character


        // the width of the cursor is always 1 character unless it is over a note column, then it is 3
        int cursorWidth = (column.type == COLUMN_NOTE ? 3 : 1) * mCharWidth + 2;
        int cursorPos = column.location * mCharWidth - 1;
        QColor cursorColor = mColorTable[COLOR_CURSOR];

        painter.fillRect(cursorPos, center, cursorWidth, mRowHeight, cursorColor);
        painter.setPen(cursorColor);
        painter.drawRect(cursorPos, center, cursorWidth - 1, mRowHeight - 1);
    }

    // text
    painter.drawPixmap(0, 0, mDisplay);


    // lines
    painter.setPen(mColorTable[COLOR_LINE]);
    for (auto lineCell : mLineCells) {
        unsigned pos = lineCell * mCharWidth;
        painter.drawLine(pos, 0, pos, h);
    }
}

void PatternGrid::resizeEvent(QResizeEvent *evt) {

    auto oldSize = evt->oldSize();
    auto newSize = evt->size();

    if (newSize.height() != oldSize.height()) {
        auto newVisible = getVisibleRows();
        if (newVisible != mVisibleRows) {
            // the number of rows visible onscreen has changed

            unsigned centerRow = mVisibleRows / 2;
            unsigned newCenter = newVisible / 2;
            if (newVisible < mVisibleRows) {
                // less rows visible, crop the old image
                mDisplay = mDisplay.copy(0, (centerRow - newCenter) * mRowHeight, mDisplay.width(), newVisible * mRowHeight);
            } else {
                // more rows to display, enlarge and paint new rows


                // example: resizing results in 9 rows visible instead of 3
                // mVisibleRows (old): 3
                // mVisibleRows (new): 9
                // newCenter = 9 / 2 = 4
                // centerRow = 3 / 2 = 1

                // 0
                // 1
                // 2
                // ------- <- newCenter - centerRow
                // 3 0
                // 4 1
                // 5 2
                // ------- <- newCenter - centerRow + mVisibleRows (old)
                // 6
                // 7
                // 8

                // draw the old image at row#3
                // draw new rows 0-3
                // draw new rows 6-9

                unsigned oldStart = (newCenter - centerRow);

                QPixmap newDisplay(mDisplay.width(), newVisible * mRowHeight);
                newDisplay.fill(Qt::transparent);
                QPainter painter(&newDisplay);
                painter.setFont(font());


                paintRows(painter, 0, oldStart);
                painter.drawPixmap(0, oldStart * mRowHeight, mDisplay);
                paintRows(painter, oldStart + mVisibleRows, newVisible);


                painter.end();

                mDisplay = newDisplay;
            }
            mVisibleRows = newVisible;

        }
    }

    if (newSize.width() != oldSize.width()) {
        // determine x offset for centering
        auto displayRect = mDisplay.rect();
        displayRect.moveCenter(rect().center());
        mDisplayXpos = displayRect.left();
    }
}

void PatternGrid::keyPressEvent(QKeyEvent *evt) {
    switch (evt->key()) {
        case Qt::Key_Left:
            cursorLeft();
            break;
        case Qt::Key_Right:
            cursorRight();
            break;
        case Qt::Key_Up:
            cursorUp();
            break;
        case Qt::Key_Down:
            cursorDown();
            break;
        case Qt::Key_PageDown:
            setCursorRow(mCursorRow + mPageStep);
            emit cursorRowChanged(mCursorRow);
            break;
        case Qt::Key_PageUp:
            setCursorRow(mCursorRow - mPageStep);
            emit cursorRowChanged(mCursorRow);
            break;
        default:
            QWidget::keyPressEvent(evt);
            break;
    }
}

void PatternGrid::mouseMoveEvent(QMouseEvent *evt) {

}

void PatternGrid::mousePressEvent(QMouseEvent *evt) {
    if (evt->button() == Qt::LeftButton) {
        //setFocus();

        mSelecting = true;

    }
}

void PatternGrid::mouseReleaseEvent(QMouseEvent *evt) {

    if (evt->button() == Qt::LeftButton) {
        int mx = evt->x(), my = evt->y();

        // translate
        mx -= mDisplayXpos;
        int rowno = 4 * mCharWidth;
        mx -= rowno;

        if (mx >= 0 && mx < mDisplay.width() - rowno) {

            unsigned row, column;
            getCursorFromMouse(mx, my, row, column);

            setCursorRow(row);
            mCursorCol = column;
            // redraw cursor
            update();
        }
    }
}

void PatternGrid::wheelEvent(QWheelEvent *evt) {

    mWheel += evt->angleDelta().y();
    int amount = 0;
    // 120 / 8 = 15 degrees
    if (mWheel >= 120) {
        mWheel -= 120;
        amount = -mPageStep;
    } else if (mWheel <= -120) {
        mWheel += 120;
        amount = mPageStep;
    }

    if (amount) {
        setCursorRow(mCursorRow + amount);
        emit cursorRowChanged(mCursorRow);
    }

    evt->accept();

}

// ======================================================= PRIVATE METHODS ===

void PatternGrid::appearanceChanged() {

    // determine character width and height
    QFontMetrics metrics(font(), &mDisplay);

    mRowHeight = metrics.height() + 2;
    mCharWidth = metrics.averageCharWidth();

    mVisibleRows = getVisibleRows();

    // repaint rows on next repaint
    mRepaintImage = true;

    // resize image
    // add 4 to the layout since row numbers are not included
    mDisplay = QPixmap((mCellLayout.size() + 4) * mCharWidth, mVisibleRows * mRowHeight);

}

void PatternGrid::scroll(int rows) {
    if (rows == 0) {
        // nothing to do
        return;
    }

    int distance = abs(rows);
    mCursorRow += rows;

    if (distance >= mVisibleRows) {
        // no need to scroll, repaint the entire thing
        mRepaintImage = true;

    } else {
        int rowStart;
        int rowEnd;

        if (rows < 0) {
            // scrolling upwards
            rowStart = 0;
            rowEnd = distance;
        } else {
            // scrolling downwards
            rowStart = mVisibleRows - distance;
            rowEnd = mVisibleRows;
        }

        mDisplay.scroll(0, -rows * mRowHeight, mDisplay.rect());

        QPainter painter(&mDisplay);
        painter.setFont(font());

        // erase the old rows
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(0, rowStart * mRowHeight, mDisplay.width(), distance * mRowHeight, Qt::transparent);

        // paint the new rows
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        paintRows(painter, rowStart, rowEnd);
        painter.end();
    }
    update();
}

void PatternGrid::getCursorFromMouse(int x, int y, unsigned &outRow, unsigned &outCol) {

    outCol = mCellLayout[x / mCharWidth];
    outRow = mCursorRow + (y / mRowHeight - (mVisibleRows / 2));

}

unsigned PatternGrid::getVisibleRows() {
    auto h = height();
    unsigned rowsVisible = h / mRowHeight;
    if (h % mRowHeight) {
        ++rowsVisible;
    }
    // always have at least 1 row visible
    return std::max(rowsVisible, 1u);
}

void PatternGrid::paintRows(QPainter &painter, int rowStart, int rowEnd) {

    unsigned ypos = rowStart * mRowHeight;

    painter.setFont(font());
    painter.setPen(mColorTable[COLOR_FG]);

    int rowAdjusted = mCursorRow - (mVisibleRows / 2) + rowStart;

    for (int row = rowStart; row < rowEnd; ++row) {

        if (rowAdjusted >= 0 && rowAdjusted < mPatternSize) {
            painter.drawText(mCharWidth, ypos, mCharWidth * 50, mRowHeight, Qt::AlignVCenter, QString("%1  ... .. ...  ... .. ...  ... .. ...  ... .. ...").arg(rowAdjusted, 2, 16, QLatin1Char('0')).toUpper());

        }
        rowAdjusted++;
        ypos += mRowHeight;
    }
}

}
