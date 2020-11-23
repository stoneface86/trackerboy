
#include "widgets/grid/PatternGrid.hpp"


#include <QFontDatabase>
#include <QPainter>

#include <QtDebug>

#include <algorithm>

// row numbers start at column 1
constexpr int COL_INDEX_ROW_NO = 1;
// track1 starts at column 5
constexpr int COL_INDEX_TRACK1 = 5;



PatternGrid::PatternGrid(OrderModel &model, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mEffectsVisible{ 1 },
    mRepaintImage(true),
    mCursorRow(0),
    mCursorCol(COL_INDEX_TRACK1),
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
    mColorTable[COLOR_BG]               = QColor(8, 24, 32);
    mColorTable[COLOR_BG_ROW]           = QColor(20, 20, 80, 128);
    mColorTable[COLOR_CURSOR]           = QColor(192, 192, 192, 128); 
    mColorTable[COLOR_FG]               = QColor(136, 192, 112);
    mColorTable[COLOR_FG_HIGHLIGHT]     = QColor(224, 248, 208);
    mColorTable[COLOR_LINE]             = QColor(64, 64, 64);

    // initialize Cell vector
    
    // typical column setup
    //
    // .RR.|.NNN.II.EEE.|.NNN.II.EEE.|.NNN.II.EEE.|.NNN.II.EEE.|
    // RR: row number
    // NNN: note
    // II: Instrument id
    // EEE: effect type + argument
    // | : line
    //
    // A track is 13, 17 or 21 chars long
    // so the maximum number of chars on one line is
    // 5 + (21 * 4) = 89

    // row numbers
    mCells.push_back({ 0, COLUMN_LINE });
    mCells.push_back({ 0, COLUMN_NONE });
    mCells.push_back({ 0, COLUMN_NONE });
    mCells.push_back({ 0, COLUMN_NONE });
    mCells.push_back({ 0, COLUMN_LINE });
    mLineCells[0] = 0;
    mLineCells[1] = 4;


    for (unsigned i = 0; i != 4; ++i) {

        //mCells.push_back({ i, COLUMN_NONE }); // spacer

        mCells.push_back({ i, COLUMN_NOTE1 }); // note column, 1 cell 3 chars
        mCells.push_back({ i, COLUMN_NOTE2 }); 
        mCells.push_back({ i, COLUMN_NOTE3 }); 

        mCells.push_back({ i, COLUMN_NONE }); // spacer

        mCells.push_back({ i, COLUMN_INSTRUMENT_HIGH });
        mCells.push_back({ i, COLUMN_INSTRUMENT_LOW });

        mCells.push_back({ i, COLUMN_NONE }); // spacer

        mCells.push_back({ i, COLUMN_EFFECT1_TYPE });
        mCells.push_back({ i, COLUMN_EFFECT1_ARG_HIGH });
        mCells.push_back({ i, COLUMN_EFFECT1_ARG_LOW });

        mCells.push_back({ i, COLUMN_NONE }); // spacer

        mCells.push_back({ i, COLUMN_LINE });
        mLineCells[static_cast<size_t>(i) + 2] = static_cast<unsigned>(mCells.size()) - 1;

    }


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
    mCursorCol = seekNextCell(mCursorCol, SeekDirection::left);
    repaint();
}

void PatternGrid::cursorRight() {
    mCursorCol = seekNextCell(mCursorCol, SeekDirection::right);
    repaint();
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
        repaint();
    } else if (row >= mPatternSize) {
        int patterns = row / mPatternSize;
        mCursorRow = row % mPatternSize;
        mCursorPattern += patterns;
        mCursorPattern %= mModel.rowCount();
        mRepaintImage = true;
        repaint();
    } else {
        scroll(row - mCursorRow);
    }
}

void PatternGrid::setCursorTrack(int track) {
    if (track < 0 || track > 3) {
        return;
    }

    unsigned index = 0;
    for (auto cell : mCells) {
        if (cell.column == COLUMN_NOTE1 && cell.track == track) {
            mCursorCol = index;
            break;
        }
        ++index;
    }
    repaint();
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
    auto &cell = mCells[mCursorCol];
    {
        // the cursor has a 1 pixel border around the width and height of a character
        // this way the cursor outline is not drawn under the character


        // the width of the cursor is always 1 character unless it is over a note column, then it is 3
        int cursorWidth = (cell.column == COLUMN_NOTE1 ? 3 : 1) * mCharWidth + 2;
        int cursorPos = mCursorCol * mCharWidth - 1;
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

        if (mx >= 0 && mx < mDisplay.width()) {

            unsigned row, column;
            getCursorFromMouse(mx, my, row, column);

            if (mCells[column].column < COLUMN_NONE) {
                setCursorRow(row);
                mCursorCol = column;
                repaint();
            }
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
    mDisplay = QPixmap(mCells.size() * mCharWidth, mVisibleRows * mRowHeight);
    //mDisplay = QImage(mCells.size() * mCharWidth, mVisibleRows * mRowHeight, QImage::Format_ARGB32_Premultiplied);

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
    repaint();
}



unsigned PatternGrid::seekNextCell(unsigned start, SeekDirection direction) {
    
    unsigned index = start;
    for (;;) {
        if (direction == SeekDirection::right) {
            if (++index == mCells.size()) {
                index = 0;
            }
        } else {
            if (index-- == 0) {
                index = mCells.size() - 1;
            }
        }

        auto column = mCells[index].column;
        
        if (column < COLUMN_SKIP) {
            break;
        }
    }

    return index;
}

void PatternGrid::getCursorFromMouse(int x, int y, unsigned &outRow, unsigned &outCol) {
    
    outCol = x / mCharWidth;
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
