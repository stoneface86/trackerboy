
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

// New Track layout (always 3 effects)
// .NNN.II.111222333. = 18 chars
// NNN - note column (3 cells)
// II - instrument columns (2 columns, 2 cells)
// 111 - effect 1 columns (3 columns, 3 cells)
// 222 - effect 2 columns (3 columns, 3 cells)
// 333 - effect 3 columns (3 columns, 3 cells)

// converts a column index -> cell index
uint8_t PatternGrid::TRACK_COLUMN_MAP[] = {
    // .NNN.II.111222333.
    //  0   12 3456789AB
    1,  // COLUMN_NOTE
    5,  // COLUMN_INSTRUMENT_HIGH
    6,  // COLUMN_INSTRUMENT_LOW
    8,  // COLUMN_EFFECT1_TYPE
    9,  // COLUMN_EFFECT1_ARG_HIGH
    10, // COLUMN_EFFECT1_ARG_LOW
    12, // COLUMN_EFFECT2_TYPE
    13, // COLUMN_EFFECT2_ARG_HIGH
    14, // COLUMN_EFFECT2_ARG_LOW
    16, // COLUMN_EFFECT3_TYPE
    17, // COLUMN_EFFECT3_ARG_HIGH
    18  // COLUMN_EFFECT3_ARG_LOW

};

// converts a cell index -> column index
uint8_t PatternGrid::TRACK_CELL_MAP[] = {
    COLUMN_NOTE,    // spacing
    COLUMN_NOTE,
    COLUMN_NOTE,
    COLUMN_NOTE,
    
    COLUMN_INSTRUMENT_HIGH, // spacing
    COLUMN_INSTRUMENT_HIGH,
    COLUMN_INSTRUMENT_LOW,

    COLUMN_EFFECT1_TYPE, // spacing
    COLUMN_EFFECT1_TYPE,
    COLUMN_EFFECT1_ARG_HIGH,
    COLUMN_EFFECT1_ARG_LOW,
    COLUMN_EFFECT2_TYPE, // spacing
    COLUMN_EFFECT2_TYPE,
    COLUMN_EFFECT2_ARG_HIGH,
    COLUMN_EFFECT2_ARG_LOW,
    COLUMN_EFFECT3_TYPE, // spacing
    COLUMN_EFFECT3_TYPE,
    COLUMN_EFFECT3_ARG_HIGH,
    COLUMN_EFFECT3_ARG_LOW,
    COLUMN_EFFECT3_ARG_LOW, // spacing
};


PatternGrid::PatternGrid(OrderModel &model, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mHeaderFont(":/icons/gridHeaderFont.bmp"),
    mRepaintImage(true),
    mPaintChoice(PaintChoice::both),
    mCursorRow(0),
    mCursorCol(0),
    mCursorPattern(0),
    mPatterns(1),
    mPatternSize(64),
    mDisplayXpos(0),
    mTrackHover(-1),
    mTrackFlags(0),
    mRowHeight(0),
    mCharWidth(0),
    mVisibleRows(0),
    mTrackWidth(0),
    mRownoWidth(0)
{
    

    connect(&model, &OrderModel::trackChanged, this, &PatternGrid::setCursorTrack);

    setMouseTracking(true);

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

    int cols = COLUMNS;
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
    setCursorColumn(track * TRACK_COLUMNS);
}

// ================================================================ EVENTS ===

void PatternGrid::paintEvent(QPaintEvent *evt) {
    Q_UNUSED(evt);

    QPainter painter(this);

    if (mPaintChoice != PaintChoice::grid) {
        // if choice is both or header

        // background
        painter.fillRect(0, 0, width(), HEADER_HEIGHT, mColorTable[COLOR_HEADER_BG]);

        
        // disabled tracks
        // TODO
        {
            QColor disabledColor = mColorTable[COLOR_HEADER_DISABLED];
            int xpos = mDisplayXpos + mRownoWidth;
            for (int i = 0; i != 4; ++i) {
                if (!!(mTrackFlags & (1 << i))) {
                    painter.fillRect(xpos, 0, mTrackWidth, HEADER_HEIGHT, disabledColor);
                }
                xpos += mTrackWidth;
            }
        }
        


        painter.setPen(mColorTable[COLOR_HEADER_FG]);
        painter.drawLine(0, HEADER_HEIGHT - 2, width(), HEADER_HEIGHT - 2);

        painter.translate(QPoint(mDisplayXpos + mRownoWidth, 0));

        

        // highlight
        if (mTrackHover != -1) {
            painter.setPen(mColorTable[COLOR_HEADER_HIGHLIGHT]);
            int trackBegin = mTrackWidth * mTrackHover;
            int trackEnd = trackBegin + mTrackWidth;
            painter.drawLine(trackBegin, HEADER_HEIGHT - 3, trackEnd, HEADER_HEIGHT - 3);
            painter.drawLine(trackBegin, HEADER_HEIGHT - 1, trackEnd, HEADER_HEIGHT - 1);

        }

        // lines + text
        painter.drawPixmap(0, 0, mHeaderPixmap);

        // draw volume meters


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

        // the cursor has a 1 pixel border around the width and height of a character
        // this way the cursor outline is not drawn under the character


        // the width of the cursor is always 1 character unless it is over a note column, then it is 3
        int cursorWidth = ((mCursorCol % TRACK_COLUMNS) == COLUMN_NOTE ? 3 : 1) * mCharWidth + 2;
        int cursorPos = columnLocation(mCursorCol) - 1;
        QColor cursorColor = mColorTable[COLOR_CURSOR];

        painter.fillRect(cursorPos, center, cursorWidth, mRowHeight, cursorColor);
        painter.setPen(cursorColor);
        painter.drawRect(cursorPos, center, cursorWidth - 1, mRowHeight - 1);

        // text
        painter.drawPixmap(0, 0, mDisplay);


        // lines
        painter.setPen(mColorTable[COLOR_LINE]);
        painter.drawLine(0, 0, 0, h);
        int xpos = mRownoWidth;
        for (int i = 0; i != 5; ++i) {
            painter.drawLine(xpos, 0, xpos, h);
            xpos += mTrackWidth;
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

void PatternGrid::leaveEvent(QEvent *evt) {
    Q_UNUSED(evt);

    setTrackHover(HOVER_NONE);
}

void PatternGrid::mouseMoveEvent(QMouseEvent *evt) {

    int mx = evt->x();
    int my = evt->y();

    if (my < HEADER_HEIGHT) {
        // translate the x coordinate so that 0 = track 1
        int translated = mx - mDisplayXpos - mRownoWidth;
        int hover = HOVER_NONE;

        if (translated > 0 && translated < mTrackWidth * 4) {
            hover = translated / mTrackWidth;
        }
        setTrackHover(hover);
        
    } else {
        setTrackHover(HOVER_NONE);
    }


}

void PatternGrid::mousePressEvent(QMouseEvent *evt) {
    if (evt->button() == Qt::LeftButton) {

        mSelecting = true;

    }
}

void PatternGrid::mouseReleaseEvent(QMouseEvent *evt) {

    if (evt->button() == Qt::LeftButton) {

        if (mTrackHover == HOVER_NONE) {

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
        } else {
            // user clicked on a track header either to mute or unmute the channel
            mTrackFlags ^= 1 << mTrackHover;
            updateHeader();

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

    mTrackWidth = TRACK_CELLS * mCharWidth;
    mRownoWidth = ROWNO_CELLS * mCharWidth;

    // repaint rows on next repaint
    mRepaintImage = true;

    // resize image
    mDisplay = QPixmap((mTrackWidth * 4) + mRownoWidth, mVisibleRows * mRowHeight);

    // draw the header
    mHeaderPixmap = QPixmap(mTrackWidth * 4 + 1, HEADER_HEIGHT);
    mHeaderPixmap.fill(Qt::transparent);
    QPainter painter(&mHeaderPixmap);
    painter.setPen(mColorTable[COLOR_HEADER_FG]);
    int x = 2;
    for (int i = 0; i != 4; ++i) {
        // draw "CH"
        painter.drawPixmap(x, 4, mHeaderFont, 0, 0, HEADER_FONT_WIDTH * 2, HEADER_FONT_HEIGHT);

        // draw line for volume meter
        //painter.drawLine(x, 22, x + mTrackWidth - 4, 22);

        //x += HEADER_FONT_WIDTH * 2;
        // draw number
        painter.drawPixmap(x + (HEADER_FONT_WIDTH * 2), 4, mHeaderFont, HEADER_FONT_WIDTH * (2 + i), 0, HEADER_FONT_WIDTH, HEADER_FONT_HEIGHT);
        x += mTrackWidth;
    }

    // draw lines
    painter.setPen(mColorTable[COLOR_LINE]);
    x = 0;
    for (int i = 0; i != 5; ++i) {
        painter.drawLine(x, 0, x, HEADER_HEIGHT);
        x += mTrackWidth;
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

int PatternGrid::columnLocation(int column) {
    int track = column / TRACK_COLUMNS;
    int coltype = column % TRACK_COLUMNS;
    return track * mTrackWidth + TRACK_COLUMN_MAP[coltype] * mCharWidth + mRownoWidth;
}

void PatternGrid::getCursorFromMouse(int x, int y, unsigned &outRow, unsigned &outCol) {
    int cell = x / mCharWidth;
    int track = cell / TRACK_CELLS;
    cell = cell % TRACK_CELLS;
    outCol = (track * TRACK_COLUMNS) + TRACK_CELL_MAP[cell];
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
            painter.drawText(mCharWidth, ypos, mTrackWidth * 4 + mRownoWidth, mRowHeight, Qt::AlignVCenter, QString("%1  ... .. ... ... ...  ... .. ... ... ...  ... .. ... ... ...  ... .. ... ... ...").arg(rowAdjusted, 2, 16, QLatin1Char('0')).toUpper());

        }
        rowAdjusted++;
        ypos += mRowHeight;
    }
}

void PatternGrid::setTrackHover(int hover) {
    if (mTrackHover != hover) {
        mTrackHover = hover;
        updateHeader();
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
