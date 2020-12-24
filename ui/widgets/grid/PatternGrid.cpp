
#include "widgets/grid/PatternGrid.hpp"
#include "widgets/grid/layout.hpp"

#include "trackerboy/note.hpp"

#include <QFontDatabase>
#include <QPainter>
#include <QtDebug>

#include <algorithm>

using namespace PatternConstants;

// The TODO list
//
// 1. (Low priority) When the widget's width is too small to fit everything, we will need to translate
//    everything so that the current track being edited is visible. (Currently just gets clipped)
// 2. Selection
//    a. User should be able to select cells via the mouse
//    b. User can drag this selection to a new track (move)
// 3. Edit actions
//    a. Cut/Copy/Paste/Paste Mix
//    b. Transpose
// 4. Keyboard controls
//    a. Note key presses - edits the note column, also previews the instrument via the renderer
//    b. Shift + Arrow keys - selects cells relative to cursor
//    c. Alt+Left/Alt+Right - move cursor column to previous/next track
//    d. Ctrl+Left/Ctrl+Right - selects previous/next pattern
//    e. Ctrl+Up/Ctrl+Down - selects previous/next instrument
// 5. Editing - editing a cell requires a redraw of the cell (erase the cell and then draw the new one)


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


PatternGrid::PatternGrid(SongListModel &model, PatternGridHeader &header, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mHeader(header),
    mPainter(font()),
    mRepaintImage(true),
    mCursorRow(0),
    mCursorCol(0),
    mCursorPattern(0),
    mOffset(0),
    mPatternPrev(),
    mPatternCurr(mModel.currentSong()->getPattern(0)),
    mPatternNext(),
    mPatternRect(),
    mSelecting(false),
    mEditMode(false),
    mSettingShowPreviews(true),
    mVisibleRows(0)
{
    
    auto &orderModel = mModel.orderModel();
    connect(&orderModel, &OrderModel::currentTrackChanged, this, &PatternGrid::setCursorTrack);
    connect(&orderModel, &OrderModel::currentPatternChanged, this, &PatternGrid::setCursorPattern);
    connect(&model, &SongListModel::currentIndexChanged, this, &PatternGrid::onSongChanged);
    connect(&orderModel, &OrderModel::patternsChanged, this, [this]() {
        setPatterns(mCursorPattern);
        redraw();
        });
    connect(&model, &SongListModel::patternSizeChanged, this,
        [this](int rows) {
            if (mCursorRow >= rows) {
                mCursorRow = rows - 1;
                emit cursorRowChanged(mCursorRow);
            }
            setPatterns(mCursorPattern);
            setPatternRect();
            redraw();
        });

    connect(&model, &SongListModel::rowsPerBeatChanged, this,
        [this](int rpb) {
            mPainter.setFirstHighlight(rpb);
            redraw();
        });
    connect(&model, &SongListModel::rowsPerMeasureChanged, this,
        [this](int rpm) {
            mPainter.setSecondHighlight(rpm);
            redraw();
        });

    auto song = mModel.currentSong();
    mPainter.setFirstHighlight(song->rowsPerBeat());
    mPainter.setSecondHighlight(song->rowsPerMeasure());

    setAutoFillBackground(true);

    // first time initialization
    fontChanged();

}

void PatternGrid::redraw() {
    mRepaintImage = true;
    update();
}

int PatternGrid::row() const {
    return mCursorRow;
}

void PatternGrid::setColors(ColorTable const& colors) {
    mPainter.setColors(colors);

    // update palette so the background is automatically drawn
    auto pal = palette();
    pal.setColor(QPalette::Window, colors[+Color::background]);
    setPalette(pal);

    // new colors, redraw everything
    redraw();
}

void PatternGrid::setPreviewEnable(bool previews) {
    if (previews != mSettingShowPreviews) {
        mSettingShowPreviews = previews;
        setPatterns(mCursorPattern);
        if (!previews) {
            mPatternPrev.reset();
            mPatternNext.reset();
        }

        redraw();
    }
}

void PatternGrid::setShowFlats(bool showFlats) {
    if (showFlats != mPainter.flats()) {
        mPainter.setFlats(showFlats);
        redraw();
    }
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

    int track = mCursorCol / TRACK_COLUMNS;
    int newtrack = column / TRACK_COLUMNS;
    if (track != newtrack) {
        mModel.orderModel().selectTrack(newtrack);
    }

    mCursorCol = column;
    update();
    emit cursorColumnChanged(mCursorCol);
}

void PatternGrid::setCursorRow(int row) {
    if (mCursorRow == row) {
        return;
    }

    auto song = mModel.currentSong();

    if (row < 0) {
        // go to the previous pattern or wrap around to the last one
        setCursorPattern(mCursorPattern == 0 
                            ? static_cast<int>(song->orders().size()) - 1 
                            : mCursorPattern - 1);
        int currCount = mPatternCurr.totalRows();
        mCursorRow = std::max(0, currCount + row);
        mPatternRect.moveTop(((mVisibleRows / 2) - mCursorRow) * mPainter.cellHeight());

        mRepaintImage = true;
        update();
    } else if (row >= static_cast<int>(mPatternCurr.totalRows())) {
        // go to the next pattern or wrap around to the first one
        int nextPattern = mCursorPattern + 1;
        setCursorPattern(nextPattern == song->orders().size() ? 0 : nextPattern);
        int currCount = mPatternCurr.totalRows();
        mCursorRow = std::min(currCount - 1, row - currCount);
        mPatternRect.moveTop(((mVisibleRows / 2) - mCursorRow) * mPainter.cellHeight());

        mRepaintImage = true;
        update();
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

    // update selected pattern in the model
    mModel.orderModel().selectPattern(pattern);

    // full repaint
    redraw();
}

void PatternGrid::setCursorTrack(int track) {
    if (track < 0 || track > 3) {
        return;
    }
    setCursorColumn(track * TRACK_COLUMNS);
}

// ================================================================ EVENTS ===

void PatternGrid::changeEvent(QEvent *evt) {
    if (evt->type() == QEvent::FontChange) {
        mPainter.setFont(font());
        fontChanged();
    }
}

void PatternGrid::paintEvent(QPaintEvent *evt) {
    Q_UNUSED(evt);
    
    QPainter painter(this);


    if (mRepaintImage) {
        //qDebug() << "Full repaint";

        mDisplay.fill(Qt::transparent);

        QPainter displayPainter(&mDisplay);
        displayPainter.setFont(font());

        paintRows(displayPainter, 0, mVisibleRows);

        mRepaintImage = false;
    }

    auto const h = height();
    //auto const w = width();
    //auto const cellHeight = mPainter.cellHeight();
    //auto const cellWidth = mPainter.cellWidth();
    auto const rownoWidth = mPainter.rownoWidth();
    auto const trackWidth = mPainter.trackWidth();
    unsigned const centerRow = mVisibleRows / 2;

    // Z-order (from back to front)
    // 1. window background (drawn by qwidget)
    // 2. row background (mBackgroundPixmap)
    // 3. current row
    // 4. selection
    // 5. cursor
    // 6. pattern text (mDisplay)
    // 7. lines
    

    // cursor row
    /*painter.setPen(mColorTable[+Color::backgroundRow]);
    painter.drawLine(0, center, w, center);
    painter.fillRect(0, center, w, cellHeight, mColorTable[+Color::backgroundRow]);
    painter.drawLine(0, center + cellHeight - 1, w, center + cellHeight - 1);*/


    // the grid is centered so translate everything else by the x offset
    if (mOffset > 0) {
        painter.translate(QPoint(mOffset, 0));
    }

    // [2] row background
    //mPainter.drawPixmap(mPainter.rownoWidth(), 0, mBackgroundPixmap);

    // [3] current row
    mPainter.drawRowBackground(painter, mEditMode ? PatternPainter::ROW_EDIT : PatternPainter::ROW_CURRENT, centerRow);

    // [4] selection
    // TODO

    // [5] cursor
    mPainter.drawCursor(painter, centerRow, mCursorCol);

    // [6] text
    if (mSettingShowPreviews) {
        // previews are drawn at 50% opacity
        // we paint the portion of the text that contains previews at 50%,
        // where the current pattern is drawn at 100%

        int heightPrev = std::max(0, mPatternRect.top());
        int heightCurr = mDisplay.height() - heightPrev;

        int yNext = mPatternRect.bottom();

        painter.setOpacity(0.5);
        if (heightPrev) {
            // previous pattern
            painter.drawPixmap(0, 0, mDisplay, 0, 0, -1, heightPrev);
        }

        if (yNext < mDisplay.height()) {
            // next pattern
            int heightNext = mDisplay.height() - yNext;
            painter.drawPixmap(0, yNext, mDisplay, 0, yNext, -1, heightNext);
            heightCurr -= heightNext;
        }
        
        painter.setOpacity(1.0);
        painter.drawPixmap(0, heightPrev, mDisplay, 0, heightPrev, -1, heightCurr);
    } else {
        painter.drawPixmap(0, 0, mDisplay);
    }

    // [7] lines
    mPainter.drawLines(painter, h);

        
}

void PatternGrid::resizeEvent(QResizeEvent *evt) {

    auto oldSize = evt->oldSize();
    auto newSize = evt->size();

    if (newSize.height() != oldSize.height()) {
        auto oldVisible = mVisibleRows;
        mVisibleRows = getVisibleRows();

        if (mVisibleRows != oldVisible) {
            // the number of rows visible onscreen has changed

            auto const cellHeight = mPainter.cellHeight();
            unsigned centerOld = oldVisible / 2;
            unsigned centerNew = mVisibleRows / 2;
            if (mVisibleRows < oldVisible) {
                // less rows visible, crop the old image
                mDisplay = mDisplay.copy(0, (centerOld - centerNew) * cellHeight, mDisplay.width(), mVisibleRows * cellHeight);
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

                unsigned oldStart = (centerNew - centerOld);

                QPixmap newDisplay(mDisplay.width(), mVisibleRows * cellHeight);
                newDisplay.fill(Qt::transparent);
                QPainter painter(&newDisplay);
                painter.setFont(font());

                paintRows(painter, 0, oldStart);
                painter.drawPixmap(0, oldStart * cellHeight, mDisplay);
                paintRows(painter, oldStart + oldVisible, mVisibleRows);


                painter.end();

                mDisplay = newDisplay;
            }
            setPatternRect();

        }
    }

    if (newSize.width() != oldSize.width()) {
        // determine x offset for centering
        calcOffset();
        mHeader.setOffset(mOffset);
    }

    
}

void PatternGrid::leaveEvent(QEvent *evt) {
    Q_UNUSED(evt);
}

void PatternGrid::mouseMoveEvent(QMouseEvent *evt) {
    Q_UNUSED(evt);
    //int mx = evt->x();
    //int my = evt->y();


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
        mx -= mOffset;
            

        if (mPatternRect.contains(mx, my)) {
            unsigned row, column;
            mx -= mPainter.rownoWidth();
            getCursorFromMouse(mx, my, row, column);

            setCursorRow(row);
            setCursorColumn(column);
            // redraw cursor
            //update();
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
        
        // select the first pattern
        setPatterns(0);
        mCursorPattern = 0;

        // update the rectangle
        setPatternRect();

        // set highlight intervals
        auto song = mModel.currentSong();
        mPainter.setFirstHighlight(song->rowsPerBeat());
        mPainter.setSecondHighlight(song->rowsPerMeasure());

        // redraw everything
        redraw();


    }
}

void PatternGrid::fontChanged() {

    mVisibleRows = getVisibleRows();
    setPatternRect();


    // repaint rows on next repaint
    mRepaintImage = true;

    // resize image
    auto const rownoWidth = mPainter.rownoWidth();
    auto const trackWidth = mPainter.trackWidth();

    mDisplay = QPixmap((trackWidth * 4) + rownoWidth, mVisibleRows * mPainter.cellHeight());
    calcOffset();

    // let the header know about these changes
    mHeader.setOffset(mOffset);
    mHeader.setWidths(rownoWidth, trackWidth);
}

void PatternGrid::calcOffset() {
    auto displayRect = mDisplay.rect();
    displayRect.moveCenter(rect().center());
    mOffset = displayRect.left();
}

void PatternGrid::scroll(int rows) {
    if (rows == 0) {
        // nothing to do
        return;
    }

    auto const cellHeight = mPainter.cellHeight();
    mPatternRect.translate(0, -rows * cellHeight);
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

        mDisplay.scroll(0, -rows * cellHeight, mDisplay.rect());

        QPainter painter(&mDisplay);
        painter.setFont(font());

        // erase the old rows
        auto mode = painter.compositionMode();
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(0, rowStart * cellHeight, mDisplay.width(), distance * cellHeight, Qt::transparent);

        // paint the new rows
        painter.setCompositionMode(mode);
        paintRows(painter, rowStart, rowEnd);
        painter.end();
    }
    update();
}


void PatternGrid::getCursorFromMouse(int x, int y, unsigned &outRow, unsigned &outCol) {
    int cell = x / mPainter.cellWidth();
    int track = cell / TRACK_CELLS;
    cell = cell % TRACK_CELLS;
    outCol = (track * TRACK_COLUMNS) + TRACK_CELL_MAP[cell];
    outRow = mCursorRow + (y / mPainter.cellHeight() - (mVisibleRows / 2));

}

unsigned PatternGrid::getVisibleRows() {
    auto h = height();
    // integer division, rounding up
    return (h - 1) / mPainter.cellHeight() + 1;
}

void PatternGrid::paintRows(QPainter &painter, int rowStart, int rowEnd) {

    int remainder = rowEnd - rowStart;
    if (remainder <= 0) {
        return;
    }

    auto const cellHeight = mPainter.cellHeight();
    int ypos = rowStart * cellHeight;

    painter.setFont(font());

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
                ypos += -prevRow * cellHeight;
                prevRow = 0;
            }

            int rowsToPaint = std::min(remainder, -rowAdjusted);
            remainder -= rowsToPaint;

            for (; rowsToPaint--; ) {
                ypos = mPainter.drawRow(painter, pattern[static_cast<uint16_t>(prevRow)], prevRow, ypos);
                prevRow++;
            }
        } else {
            // no previous pattern (mCursorPattern == 0)
            // just skip these rows
            remainder += rowAdjusted;
            ypos += cellHeight * -rowAdjusted;
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

        for (; rowsToPaint--; ) {
            ypos = mPainter.drawRow(painter, mPatternCurr[static_cast<uint16_t>(rowAdjusted)], rowAdjusted, ypos);
            rowAdjusted++;
        }

        if (remainder == 0) {
            return;
        }
    }

    if (mPatternNext) {
        auto &pattern = mPatternNext.value();

        int nextRow = rowAdjusted - patternSize;
        int nextPatternSize = pattern.totalRows();
        if (nextRow < nextPatternSize) {
            int rowsToPaint = std::min(remainder, nextPatternSize - nextRow);
            assert(rowsToPaint >= 0);
            for (; rowsToPaint--; ) {
                ypos = mPainter.drawRow(painter, pattern[static_cast<uint16_t>(nextRow)], nextRow, ypos);
                ++nextRow;
            }
        }
    }

}

void PatternGrid::setPatterns(int pattern) {
    Q_ASSERT(pattern >= 0 && pattern < 256);
    auto song = mModel.currentSong();

    
    if (mSettingShowPreviews) {
        // get the previous pattern for preview
        if (pattern > 0) {
            mPatternPrev.emplace(song->getPattern(static_cast<uint8_t>(pattern) - 1));
        } else {
            mPatternPrev.reset();
        }
        // get the next pattern for preview
        auto nextPattern = pattern + 1;
        if (nextPattern < song->orders().size()) {
            mPatternNext.emplace(song->getPattern(static_cast<uint8_t>(nextPattern)));
        } else {
            mPatternNext.reset();
        }
    }

    // update the current pattern
    mPatternCurr = song->getPattern(static_cast<uint8_t>(pattern));

    
}

void PatternGrid::setPatternRect() {
    auto const cellHeight = mPainter.cellHeight();

    mPatternRect.setX(mPainter.rownoWidth());
    mPatternRect.setY(((mVisibleRows / 2) - mCursorRow) * cellHeight);
    mPatternRect.setWidth(mPainter.trackWidth() * 4);
    mPatternRect.setHeight(cellHeight * mPatternCurr.totalRows());
}
