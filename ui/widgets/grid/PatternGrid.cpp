
#include "widgets/grid/PatternGrid.hpp"

#include "trackerboy/note.hpp"

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

// Performance notes
// Repeatedly doing full repaints results in 5-9% CPU Usage
// Debug build, Windows 10, Ryzen 2600 @ 3.4 GHz
//
// On release build doing the same results in 1-3% CPU Usage
//
// Always profile Release before considering an optimization


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
    11, // COLUMN_EFFECT2_TYPE
    12, // COLUMN_EFFECT2_ARG_HIGH
    13, // COLUMN_EFFECT2_ARG_LOW
    14, // COLUMN_EFFECT3_TYPE
    15, // COLUMN_EFFECT3_ARG_HIGH
    16  // COLUMN_EFFECT3_ARG_LOW

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
    //COLUMN_EFFECT2_TYPE, // spacing
    COLUMN_EFFECT2_TYPE,
    COLUMN_EFFECT2_ARG_HIGH,
    COLUMN_EFFECT2_ARG_LOW,
    //COLUMN_EFFECT3_TYPE, // spacing
    COLUMN_EFFECT3_TYPE,
    COLUMN_EFFECT3_ARG_HIGH,
    COLUMN_EFFECT3_ARG_LOW,
    COLUMN_EFFECT3_ARG_LOW, // spacing
};

static const char HEX_TABLE[16] = {
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'A',
    'B',
    'C',
    'D',
    'E',
    'F'
};

// TODO: move this somewhere in the library
static char effectTypeToChar(trackerboy::EffectType et) {

    switch (et) {
        case trackerboy::EffectType::patternGoto:
            return 'B';
        case trackerboy::EffectType::patternHalt:
            return 'C';
        case trackerboy::EffectType::patternSkip:
            return 'D';
        case trackerboy::EffectType::setTempo:
            return 'F';
        case trackerboy::EffectType::sfx:
            return 'T';
        case trackerboy::EffectType::setEnvelope:
            return 'E';
        case trackerboy::EffectType::setTimbre:
            return 'V';
        case trackerboy::EffectType::setPanning:
            return 'I';
        case trackerboy::EffectType::setSweep:
            return 'H';
        case trackerboy::EffectType::delayedCut:
            return 'S';
        case trackerboy::EffectType::delayedNote:
            return 'G';
        case trackerboy::EffectType::lock:
            return 'L';
        case trackerboy::EffectType::arpeggio:
            return '0';
        case trackerboy::EffectType::pitchUp:
            return '1';
        case trackerboy::EffectType::pitchDown:
            return '2';
        case trackerboy::EffectType::autoPortamento:
            return '3';
        case trackerboy::EffectType::vibrato:
            return '4';
        case trackerboy::EffectType::vibratoDelay:
            return '5';
        case trackerboy::EffectType::tuning:
            return 'P';
        case trackerboy::EffectType::noteSlideUp:
            return 'Q';
        case trackerboy::EffectType::noteSlideDown:
            return 'R';
        default:
            return '?';
    }
}


PatternGrid::PatternGrid(SongListModel &model, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mHeaderFont(":/icons/gridHeaderFont.bmp"),
    mRepaintImage(true),
    mCursorRow(0),
    mCursorCol(0),
    mCursorPattern(0),
    mPatternPrev(),
    mPatternCurr(mModel.currentSong()->getPattern(0)),
    mPatternNext(),
    mDisplayXpos(0),
    mTrackHover(-1),
    mTrackFlags(0),
    mRowHeight(0),
    mCharWidth(0),
    mVisibleRows(0),
    mTrackWidth(0),
    mRownoWidth(0)
{
    
    auto orderModel = mModel.orderModel();
    connect(orderModel, &OrderModel::trackChanged, this, &PatternGrid::setCursorTrack);
    connect(orderModel, &OrderModel::patternChanged, this, &PatternGrid::setCursorPattern);
    connect(&model, &SongListModel::currentIndexChanged, this, &PatternGrid::onSongChanged);

    setMouseTracking(true);

    // default colors
    mColorTable[COLOR_BG] = QColor(8, 24, 32);
    mColorTable[COLOR_BG_ROW] = QColor(20, 20, 80, 128);
    mColorTable[COLOR_CURSOR] = QColor(192, 192, 192, 128);
    mColorTable[COLOR_FG] = QColor(136, 192, 112);
    mColorTable[COLOR_FG_HIGHLIGHT] = QColor(224, 248, 208);
    mColorTable[COLOR_FG_INSTRUMENT] = QColor(16, 16, 240);
    mColorTable[COLOR_FG_EFFECT_TYPE] = QColor(0, 200, 200);
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

    auto song = mModel.currentSong();

    if (row < 0) {
        // go to the previous pattern or wrap around to the last one
        setCursorPattern(mCursorPattern == 0 ? song->orders().size() - 1 : mCursorPattern - 1);
        int currCount = mPatternCurr.totalRows();
        mCursorRow = std::max(0, currCount + row);

        mRepaintImage = true;
        updateGrid();
    } else if (row >= mPatternCurr.totalRows()) {
        // go to the next pattern or wrap around to the first one
        int nextPattern = mCursorPattern + 1;
        setCursorPattern(nextPattern == song->orders().size() ? 0 : nextPattern);
        int currCount = mPatternCurr.totalRows();
        mCursorRow = std::min(currCount - 1, row - currCount);

        mRepaintImage = true;
        updateGrid();
    } else {
        scroll(row - mCursorRow);
    }
    emit cursorRowChanged(mCursorRow);
}

void PatternGrid::setCursorPattern(int pattern) {
    if (mCursorPattern == pattern) {
        return;
    }

    setPatterns(pattern);

    mCursorRow = std::min(mCursorRow, static_cast<int>(mPatternCurr.totalRows()));
    mCursorPattern = pattern;
    // full repaint
    mRepaintImage = true;
    updateGrid();
    emit cursorPatternChanged(pattern);
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

    // HEADER ================================================================

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
        //painter.drawLine(trackBegin, HEADER_HEIGHT - 3, trackEnd, HEADER_HEIGHT - 3);
        painter.drawLine(trackBegin, HEADER_HEIGHT - 1, trackEnd, HEADER_HEIGHT - 1);

    }

    // lines + text
    painter.drawPixmap(0, 0, mHeaderPixmap);

    // draw volume meters


    painter.resetTransform();

    // GRID ==================================================================

    if (mRepaintImage) {
        qDebug() << "Full repaint";

        mDisplay.fill(Qt::transparent);

        QPainter displayPainter(&mDisplay);
        displayPainter.setFont(font());

        paintRows(displayPainter, 0, mVisibleRows);

        mRepaintImage = false;
    }

    auto const h = height() - HEADER_HEIGHT;
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

void PatternGrid::onSongChanged(int index) {
    // -1 only occurs when the model is being reset
    if (index != -1) {
        if (mCursorRow != 0) {
            mCursorRow = 0;
            emit cursorRowChanged(0);
        }

        if (mCursorCol != 0) {
            mCursorCol = 0;
            emit cursorColumnChanged(0);
        }
        
        setPatterns(0);
        mCursorPattern = 0;

        mRepaintImage = true;
        updateGrid();


    }
}

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
        auto mode = painter.compositionMode();
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(0, rowStart * mRowHeight, mDisplay.width(), distance * mRowHeight, Qt::transparent);

        // paint the new rows
        painter.setCompositionMode(mode);
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
    if (h <= 0) {
        return 1;
    }
    // integer division, rounding up
    return (h - 1) / mRowHeight + 1;
}

void PatternGrid::paintRows(QPainter &painter, int rowStart, int rowEnd) {

    unsigned ypos = rowStart * mRowHeight;

    painter.setFont(font());
    painter.setPen(mColorTable[COLOR_FG]);

    // adjust the range of rows relative to the cursor
    // r < 0: the row from the previous pattern will be display

    int remainder = rowEnd - rowStart;

    //
    // adjusted row index, r
    // r < 0                : we paint from the previous pattern, such that -1 is the last row in the pattern
    // 0 <= r < patternSize : we paint from the current pattern
    // r >= patternSize     : we paint from the next pattern
    //
    int rowAdjusted = mCursorRow - (mVisibleRows / 2) + rowStart;

    

    if (rowAdjusted < 0) {
        // paint the previous pattern

        if (mPatternPrev) {

            auto &pattern = mPatternPrev.value();

            int prevRow = rowAdjusted + pattern.totalRows();
            if (prevRow < 0) {
                // skip these rows
                rowAdjusted -= prevRow;
                remainder += prevRow;
                if (remainder <= 0) {
                    return;
                }
                ypos += -prevRow * mRowHeight;
                prevRow = 0;
            }

            int rowsToPaint = std::min(remainder, -rowAdjusted);
            remainder -= rowsToPaint;
            painter.setOpacity(0.5);

            for (; rowsToPaint--; ) {
                paintRow(painter, pattern[prevRow], prevRow, ypos);
                prevRow++;
                ypos += mRowHeight;
            }
        } else {
            // no previous pattern (mCursorPattern == 0)
            // just skip these rows
            remainder += rowAdjusted;
            ypos += mRowHeight * -rowAdjusted;
        }

        // stop here if there are no more rows to paint
        if (remainder <= 0) {
            return;
        }
        rowAdjusted = 0;
    }

    // at this point rowAdjusted is >= 0
    int const patternSize = mPatternCurr.totalRows();

    if (rowAdjusted < patternSize) {
        int rowsToPaint = std::min(remainder, patternSize - rowAdjusted);
        remainder -= rowsToPaint;

        painter.setOpacity(1.0);
        for (; rowsToPaint--; ) {
            paintRow(painter, mPatternCurr[rowAdjusted], rowAdjusted, ypos);
            rowAdjusted++;
            ypos += mRowHeight;
        }

        if (remainder == 0) {
            return;
        }
    }

    if (mPatternNext) {
        auto &pattern = mPatternNext.value();

        int nextRow = rowAdjusted - patternSize;
        int rowsToPaint = std::min(remainder, static_cast<int>(pattern.totalRows()) - nextRow);
        painter.setOpacity(0.5);
        for (; rowsToPaint--; ) {
            paintRow(painter, pattern[nextRow], nextRow, ypos);
            ++nextRow;
            ypos += mRowHeight;
        }
    }

}

void PatternGrid::paintRow(QPainter &painter, trackerboy::PatternRow rowdata, int rowno, int ypos) {
    QPen fgpen(mColorTable[
        (rowno % mModel.currentSong()->rowsPerBeat()) == 0 ? COLOR_FG_HIGHLIGHT : COLOR_FG
    ]);
    
    painter.setPen(fgpen);
    painter.drawText(mCharWidth, ypos, mCharWidth * 2, mRowHeight, Qt::AlignVCenter, QString("%1").arg(rowno, 2, 16, QLatin1Char('0')).toUpper());
    int xpos = (TRACK_COLUMN_MAP[COLUMN_NOTE] + ROWNO_CELLS) * mCharWidth;
    for (int track = 0; track != 4; ++track) {
        auto &trackdata = rowdata[track];

        if (!!(trackdata.flags & trackerboy::TrackRow::COLUMN_NOTE)) {
            paintNote(painter, trackdata.note, xpos, ypos);
        } else {
            paintNone(painter, 3, xpos, ypos);
        }


        xpos += (TRACK_COLUMN_MAP[COLUMN_INSTRUMENT_HIGH] - TRACK_COLUMN_MAP[COLUMN_NOTE]) * mCharWidth;
        if (!!(trackdata.flags & trackerboy::TrackRow::COLUMN_INST)) {
            uint8_t inst = trackdata.instrumentId;
            painter.setPen(mColorTable[COLOR_FG_INSTRUMENT]);
            paintCell(painter, HEX_TABLE[inst >> 4], xpos, ypos);
            paintCell(painter, HEX_TABLE[inst & 0xF], xpos + mCharWidth, ypos);
            painter.setPen(fgpen);
        } else {
            paintNone(painter, 2, xpos, ypos);
        }

        xpos += (TRACK_COLUMN_MAP[COLUMN_EFFECT1_TYPE] - TRACK_COLUMN_MAP[COLUMN_INSTRUMENT_HIGH]) * mCharWidth;
        int effectFlag = trackerboy::TrackRow::COLUMN_EFFECT1;
        for (int effect = 0; effect < trackerboy::TrackRow::MAX_EFFECTS; ++effect) {
            if (!!(trackdata.flags & effectFlag)) {
                auto effectdata = trackdata.effects[effect];
                painter.setPen(mColorTable[COLOR_FG_EFFECT_TYPE]);

                paintCell(painter, effectTypeToChar(effectdata.type), xpos, ypos);
                xpos += mCharWidth;

                painter.setPen(fgpen);
                paintCell(painter, HEX_TABLE[effectdata.param >> 4], xpos, ypos);
                xpos += mCharWidth;
                paintCell(painter, HEX_TABLE[effectdata.param & 0xF], xpos, ypos);
                xpos += mCharWidth;
            } else {
                paintNone(painter, 3, xpos, ypos);
                xpos += mCharWidth * 3;
            }

            effectFlag <<= 1;
            
        }

        xpos += 2 * mCharWidth;
    }
}

void PatternGrid::paintNone(QPainter &painter, int cells, int xpos, int ypos) {
    xpos += 3;
    ypos += mRowHeight / 2;
    int const width = mCharWidth - 6;
    for (int i = cells; i--; ) {
        painter.drawLine(xpos, ypos, xpos + width, ypos);
        xpos += mCharWidth;
    }
}

void PatternGrid::paintNote(QPainter &painter, uint8_t note, int xpos, int ypos) {

    const char* NOTE_STR_SHARP[] = {
        "C-",
        "C#",
        "D-",
        "D#",
        "E-",
        "F-",
        "F#",
        "G-",
        "G#",
        "A-",
        "A#",
        "B-"
    };

    const char* NOTE_STR_FLAT[] = {
        "C-",
        "Db",
        "D-",
        "Eb",
        "E-",
        "F-",
        "Gb",
        "G-",
        "Ab",
        "A-",
        "Bb",
        "B-"
    };

    //painter.setPen(mColorTable[COLOR_FG]);

    if (note == trackerboy::NOTE_CUT) {
        painter.setBrush(QBrush(painter.pen().color()));
        painter.drawRect(xpos, ypos + mRowHeight / 2, mCharWidth * 2, 2);
    } else {

        int octave = note / 12;
        int key = note % 12;
        octave += 2;

        painter.drawText(xpos, ypos, 3 * mCharWidth, mRowHeight, Qt::AlignVCenter,
            QString("%1%2").arg(NOTE_STR_SHARP[key]).arg(octave));
    }
}

void PatternGrid::paintCell(QPainter &painter, char cell, int xpos, int ypos) {
    painter.drawText(xpos, ypos, mCharWidth, mRowHeight, Qt::AlignVCenter, QString(cell));
}

void PatternGrid::eraseCells(QPainter &painter, int cells, int xpos, int ypos) {
    // save the current composition mode
    auto compMode = painter.compositionMode();

    // replace all pixels in the cells with transparent pixels
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(xpos, ypos, cells * mCharWidth, mRowHeight, Qt::transparent);

    // restore previous composition mode
    painter.setCompositionMode(compMode);
}

void PatternGrid::setTrackHover(int hover) {
    if (mTrackHover != hover) {
        mTrackHover = hover;
        updateHeader();
    }
}

void PatternGrid::setPatterns(int pattern) {
    auto song = mModel.currentSong();

    // get the previous pattern for preview
    if (pattern > 0) {
        mPatternPrev.emplace(song->getPattern(pattern - 1));
    } else {
        mPatternPrev.reset();
    }

    // update the current pattern
    mPatternCurr = song->getPattern(pattern);

    // get the next pattern for preview
    int nextPattern = pattern + 1;
    if (nextPattern < song->orders().size()) {
        mPatternNext.emplace(song->getPattern(nextPattern));
    } else {
        mPatternNext.reset();
    }
}

void PatternGrid::updateHeader() {
    update(QRect(0, 0, width(), HEADER_HEIGHT));
}

void PatternGrid::updateGrid() {
    update(QRect(0, HEADER_HEIGHT, width(), height() - HEADER_HEIGHT));
}

void PatternGrid::updateAll() {
    update();
}
