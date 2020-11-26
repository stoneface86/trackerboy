
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

constexpr int HEADER_HEIGHT = 32;

PatternGrid::PatternGrid(OrderModel &model, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mHeaderFont(":/icons/gridHeaderFont.bmp"),
    mEffectsVisible{ 1, 1, 1, 1 },
    mRepaintImage(true),
    mPaintChoice(PaintChoice::both),
    mCursorRow(0),
    mCursorCol(0),
    mCursorPattern(0),
    mPatterns(1),
    mPatternSize(64),
    mDisplayXpos(0),
    mRowHeight(0),
    mCharWidth(0),
    mVisibleRows(0)
{
    

    connect(&model, &OrderModel::trackChanged, this, &PatternGrid::setCursorTrack);


    // default colors
    mColorTable[COLOR_BG] = QColor(8, 24, 32);
    mColorTable[COLOR_BG_ROW] = QColor(20, 20, 80, 128);
    mColorTable[COLOR_CURSOR] = QColor(192, 192, 192, 128);
    mColorTable[COLOR_FG] = QColor(136, 192, 112);
    mColorTable[COLOR_FG_HIGHLIGHT] = QColor(224, 248, 208);
    mColorTable[COLOR_LINE] = QColor(64, 64, 64);

    mColorTable[COLOR_HEADER_BG] = mColorTable[COLOR_FG_HIGHLIGHT];
    mColorTable[COLOR_HEADER_FG] = mColorTable[COLOR_BG];
    mColorTable[COLOR_HEADER_HIGHLIGHT] = mColorTable[COLOR_FG];
    mColorTable[COLOR_HEADER_DISABLED] = QColor(52, 104, 86);

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

        //mLineCells[static_cast<size_t>(i) + 2] = static_cast<unsigned>(mCellLayout.size() + 4);

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

void PatternGrid::moveCursorRow(int amount) {
    setCursorRow(mCursorRow + amount);
}

void PatternGrid::moveCursorColumn(int amount) {
    setCursorColumn(mCursorCol + amount);
}

void PatternGrid::setCursorColumn(int column) {
    if (mCursorCol == column) {
        return;
    }

    int cols = static_cast<int>(mColumns.size());
    if (column < 0) {
        column = cols - (-column % cols);
    } else if (column >= cols) {
        column = column % cols;
    }

    mCursorCol = column;
    updateGrid();
    emit cursorColumnChanged(mCursorCol);
}

void PatternGrid::setCursorRow(int row) {
    if (mCursorRow == row) {
        return;
    }

    if (row < 0) {
        row = -row;
        int patterns = (row / mPatternSize) + 1;
        mCursorRow = mPatternSize - (row % mPatternSize);
        mCursorPattern -= patterns;
        mCursorPattern %= mModel.rowCount();
        mRepaintImage = true;
        updateGrid();
    } else if (row >= mPatternSize) {
        int patterns = row / mPatternSize;
        mCursorRow = row % mPatternSize;
        mCursorPattern += patterns;
        mCursorPattern %= mModel.rowCount();
        mRepaintImage = true;
        updateGrid();
    } else {
        scroll(row - mCursorRow);
    }
    emit cursorRowChanged(mCursorRow);
}

void PatternGrid::setCursorTrack(int track) {
    if (track < 0 || track > 3) {
        return;
    }

    int index = 0;
    for (auto col : mColumns) {
        if (col.type == COLUMN_NOTE && col.track == track) {
            if (mCursorCol != index) {
                mCursorCol = index;
                emit cursorColumnChanged(index);
            }
            break;
        }
        ++index;
    }
    updateGrid();
}

// ================================================================ EVENTS ===

void PatternGrid::paintEvent(QPaintEvent *evt) {
    (void)evt;

    QPainter painter(this);

    if (mPaintChoice != PaintChoice::grid) {
        // if choice is both or header

        // background
        painter.fillRect(0, 0, width(), HEADER_HEIGHT, mColorTable[COLOR_HEADER_BG]);

        // disabled tracks
        // TODO

        // highlight



        painter.setPen(mColorTable[COLOR_HEADER_FG]);
        painter.drawLine(0, HEADER_HEIGHT - 2, width(), HEADER_HEIGHT - 2);

        painter.translate(QPoint(mDisplayXpos, 0));

        // lines + text
        //painter.translate(mMetrics.xoffset, 0);
        painter.drawPixmap(0, 0, mHeaderPixmap);

        // draw buttons


        // draw volume meters

        // lines
        
        painter.setPen(mColorTable[COLOR_LINE]);
        for (auto pos : mLines) {
            painter.drawLine(pos, 0, pos, HEADER_HEIGHT);
        }

        painter.resetTransform();

    }

    if (mPaintChoice != PaintChoice::header) {
        // if choice is both or grid

        if (mRepaintImage) {
            mDisplay.fill(Qt::transparent);

            QPainter displayPainter(&mDisplay);
            displayPainter.setFont(font());

            paintRows(displayPainter, 0, mVisibleRows);

            mRepaintImage = false;
        }

        auto const h = height();
        auto const w = width();
        unsigned const center = mVisibleRows / 2 * mRowHeight;

        
        painter.translate(QPoint(0, HEADER_HEIGHT));

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
        painter.drawLine(0, 0, 0, h);
        for (auto pos : mLines) {
            painter.drawLine(pos, 0, pos, h);
        }
    }

    // paint everything by default for next time
    mPaintChoice = PaintChoice::both;
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

void PatternGrid::mouseMoveEvent(QMouseEvent *evt) {
    (void)evt;
}

void PatternGrid::mousePressEvent(QMouseEvent *evt) {
    if (evt->button() == Qt::LeftButton) {

        mSelecting = true;

    }
}

void PatternGrid::mouseReleaseEvent(QMouseEvent *evt) {

    if (evt->button() == Qt::LeftButton) {
        int mx = evt->x(), my = evt->y();

        // translate
        mx -= mDisplayXpos;
        my -= HEADER_HEIGHT;
        int rowno = 4 * mCharWidth;
        mx -= rowno;

        if (mx >= 0 && mx < mDisplay.width() - rowno) {

            unsigned row, column;
            getCursorFromMouse(mx, my, row, column);

            setCursorRow(row);
            setCursorColumn(column);
            // redraw cursor
            //update();
        }
    }
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

    // calculate line positions
    int pos = mCharWidth * 4;
    mLines[0] = pos;
    int index = 1;
    for (auto effectsVisible : mEffectsVisible) {
        pos += mCharWidth * (8 + (4 * effectsVisible));
        mLines[index++] = pos;
    }

}

void PatternGrid::scroll(int rows) {
    if (rows == 0) {
        // nothing to do
        return;
    }

    unsigned distance = abs(rows);
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
    updateGrid();
}

void PatternGrid::getCursorFromMouse(int x, int y, unsigned &outRow, unsigned &outCol) {

    outCol = mCellLayout[x / mCharWidth];
    outRow = mCursorRow + (y / mRowHeight - (mVisibleRows / 2));

}

unsigned PatternGrid::getVisibleRows() {
    auto h = height() - HEADER_HEIGHT;
    if (h < 0) {
        return 1;
    }
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

void PatternGrid::updateHeader() {
    mPaintChoice = PaintChoice::header;
    update(QRect(0, 0, width(), HEADER_HEIGHT));
}

void PatternGrid::updateGrid() {
    mPaintChoice = PaintChoice::grid;
    update(QRect(0, HEADER_HEIGHT, width(), height() - HEADER_HEIGHT));
}

void PatternGrid::updateAll() {
    mPaintChoice = PaintChoice::both;
    update();
}
