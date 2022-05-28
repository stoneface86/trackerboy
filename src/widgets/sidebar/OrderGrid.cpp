
#include "widgets/sidebar/OrderGrid.hpp"
#include "utils/utils.hpp"
#include "verdigris/wobjectimpl.h"

#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QtDebug>

#include <algorithm>

// layout of the order grid
// _XX_|0110220330440
// XX: row number, 2 cells
// _: spacing
// |: line
// 11,22,33,44: track 1,2,3,4 id, 2 cells each
// 0: empty cell used as spacer

W_OBJECT_IMPL(OrderGrid)


OrderGrid::OrderGrid(PatternModel &model, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mCellPainter(),
    mLineColor(),
    mRownoColor(),
    mTextColor(),
    mPen(),
    mGridRect(),
    mVisibleRows(1),
    mHighNibble(false),
    mChangeAll(false),
    mPatternStart(0),
    mPatternEnd(0)
{
    setAutoFillBackground(true);
    setFocusPolicy(Qt::ClickFocus);
    fontChanged();
    updatePatternRange();

    connect(&model, &PatternModel::cursorChanged, this,
        [this](PatternModel::CursorChangeFlags flags) {
            if (!mChangeAll && (flags & (PatternModel::CursorTrackChanged))) {
                update();
            }
        });

    auto updateRangeAndWidget = [this]() {
            updatePatternRange();
            update();
        };
    connect(&model, &PatternModel::cursorPatternChanged, this, updateRangeAndWidget);
    connect(&model, &PatternModel::patternCountChanged, this, updateRangeAndWidget);
    
    connect(&model, &PatternModel::playingChanged, this, qOverload<>(&OrderGrid::update));
    connect(&model, &PatternModel::trackerCursorPatternChanged, this, qOverload<>(&OrderGrid::update));
    connect(&model, &PatternModel::invalidated, this, qOverload<>(&OrderGrid::update));
}

void OrderGrid::setColors(Palette const& colors) {
    auto pal = palette();
    pal.setColor(backgroundRole(), colors[Palette::ColorBackground]);
    setPalette(pal);

    mLineColor = colors[Palette::ColorLine];
    mRownoColor = colors[Palette::ColorForeground];
    mTextColor = colors[Palette::ColorForegroundHighlight1];
    mRowColor = colors[Palette::ColorRow];
    mTrackerColor = colors[Palette::ColorRowPlayer];
    mCursorColor = colors[Palette::ColorCursor];
    mCursorColor.setAlpha(128);

    update();
}

void OrderGrid::setChangeAll(bool changeAll) {
    if (mChangeAll != changeAll) {
        mChangeAll = changeAll;
        update();
    }
}

void OrderGrid::decrement() {
    incDec(-1);
}

void OrderGrid::increment() {
    incDec(1);
}

void OrderGrid::incDec(int amount) {
    auto row = mModel.currentOrderRow();

    auto addId = [amount](uint8_t &id) {
        id = (uint8_t)std::clamp((int)id + amount, 0, 0xFF);
    };
    
    if (mChangeAll) {
        std::for_each(row.begin(), row.end(), addId);
    } else {
        addId(row[mModel.cursorTrack()]);
    }

    mModel.setOrderRow(row);
}

void OrderGrid::changeEvent(QEvent *evt) {
    if (evt->type() == QEvent::FontChange) {
        fontChanged();
        update();
    }
}

void OrderGrid::keyPressEvent(QKeyEvent *evt) {

    auto key = evt->key();

    // TODO: Alt+Up/Down for moving the current pattern up/down

    switch (key) {
        case Qt::Key_Left:
            if (!mChangeAll && mHighNibble) {
                mModel.moveCursorTrack(-1);
            }
            mHighNibble = !mHighNibble;
            update();
            return;
        case Qt::Key_Right:
            if (!mChangeAll && !mHighNibble) {
                mModel.moveCursorTrack(1);
            }
            mHighNibble = !mHighNibble;
            update();
            return;
        case Qt::Key_Up: {
            auto prev = mModel.cursorPattern() - 1;
            if (prev >= 0) {
                setCursorPattern(prev);
            }
            return;
        }
        case Qt::Key_Down: {
            auto next = mModel.cursorPattern() + 1;
            if (next < mModel.patterns()) {
                setCursorPattern(next);
            }
            return;
        }
    }

    // check if hex digit was pressed
    auto hex = keyToHex(key);
    if (hex) {
        auto row = mModel.currentOrderRow();
        if (mChangeAll) {
            for (auto &id : row) {
                id = replaceNibble(id, *hex, mHighNibble);
            }
        } else {
            auto &id = row[mModel.cursorTrack()];
            id = replaceNibble(id, *hex, mHighNibble);
        }
        mModel.setOrderRow(row);
        return;
    }

    // we didn't act on the key, let QWidget handle it
    QWidget::keyPressEvent(evt);

}

void OrderGrid::mousePressEvent(QMouseEvent *evt) {
    if (evt->button() == Qt::LeftButton) {
        auto pos = evt->pos();
        if (mGridRect.contains(pos)) {
            mMousePressedPos = pos;
        }
    }
}

void OrderGrid::mouseMoveEvent(QMouseEvent *evt) {
    if (mMousePressedPos) {
        auto pos = evt->pos();
        if ((pos - *mMousePressedPos).manhattanLength() < QApplication::startDragDistance()) {
            return;
        }

        // TODO: do a drag


        mMousePressedPos.reset();
    }
}

void OrderGrid::mouseReleaseEvent(QMouseEvent *evt) {
    Q_UNUSED(evt)

    if (mMousePressedPos) {
        auto cursorPattern = mModel.cursorPattern();
        auto mousePattern = (mMousePressedPos->y() / mCellPainter.cellHeight()) + mPatternStart;

        if (mousePattern >= mPatternStart && mousePattern < mPatternEnd) {
            if (mousePattern != cursorPattern) {
                setCursorPattern(mousePattern);
            }

            // determine cell
            // -11-22-33-44-
            // the grid has 13 cells per row
            int x = mMousePressedPos->x() - mGridRect.x();
            // divide by the cell width to get the cell under the mouse
            auto const cellWidth = mCellPainter.cellWidth();
            int cell = x / cellWidth;
            // each track has 3 cells, divide by 3 to get the track
            int track = cell / 3;
            int nibble = cell % 3;
            // when nibble is:
            //  0: spacer cell
            //  1: high nibble
            //  2: low nibble
            bool highNibble;
            if (nibble == 0) {
                // user pressed a spacer cell, determine which adjacent cell
                // to select
                switch (track) {
                    case 0:
                        // -11-22-33-44-
                        // ^
                        highNibble = true;
                        break;
                    case 1:
                    case 2:
                    case 3: {
                        // -11-22-33-44-
                        //    ^  ^  ^
                        int xInCell = x - (cell * cellWidth);
                        // if the x was >= half of a cell, select the upper nibble
                        // otherwise, select the lower nibble in the previous track
                        highNibble = xInCell >= cellWidth / 2;
                        if (!highNibble) {
                            --track;
                        }
                        break;
                    }
                    default:
                        // -11-22-33-44-
                        //             ^
                        // select the last cell
                        track = 3;
                        highNibble = false;

                }
            } else {
                highNibble = nibble == 1;
            }

            mModel.setCursorTrack(track);
            if (highNibble != mHighNibble) {
                mHighNibble = highNibble;
                update();
            }
        }
        

        mMousePressedPos.reset();
    }
}

void OrderGrid::paintEvent(QPaintEvent *evt) {
    Q_UNUSED(evt)

    QPainter painter(this);

    auto const cellWidth = mCellPainter.cellWidth();
    auto const cellHeight = mCellPainter.cellHeight();
    auto const cursorPattern = mModel.cursorPattern();
    auto const& order = mModel.order();
    auto const _hasFocus = hasFocus();

    int cursorYpos = (cursorPattern - mPatternStart) * cellHeight;

    if (mModel.isPlaying()) {
        auto trackerPos = mModel.trackerCursorPattern();
        // draw the tracker position except when the editor has focus and
        // the tracker position is equal to the cursor
        if (trackerPos != cursorPattern || !_hasFocus) {
            painter.fillRect(
                mGridRect.x(),
                (trackerPos - mPatternStart) * cellHeight,
                mGridRect.width(),
                cellHeight,
                mTrackerColor
            );
        }
    }

    // cursor row background
    if (_hasFocus) {
        painter.fillRect(mGridRect.x(), cursorYpos, mGridRect.width(), cellHeight, mRowColor);
    }

    // cursor
    if (mChangeAll) {
        int xpos = mGridRect.x() + cellWidth;
        if (!mHighNibble) {
            xpos += cellWidth;
        }
        int gap = cellWidth * 3;
        for (int i = 0; i < 4; ++i) {
            painter.fillRect(xpos, cursorYpos, cellWidth, cellHeight, mCursorColor);
            xpos += gap;
        }
    
    } else {
        painter.fillRect(cursorX(), cursorYpos, cellWidth, cellHeight, mCursorColor);
    }

    int ypos = 0;
    for (int i = mPatternStart; i < mPatternEnd; ++i) {
        painter.setPen(mPen.get(mRownoColor));
        // rowno
        mCellPainter.drawHex(painter, i, SPACING, ypos);

        // order data
        painter.setPen(mPen.get(mTextColor));
        auto const row = order[i];
        int xpos = mGridRect.x();
        for (auto id : row) {
            xpos += cellWidth;
            xpos = mCellPainter.drawHex(painter, id, xpos, ypos);
        }

        ypos += cellHeight;
    }


    // line
    painter.fillRect(mGridRect.x() - LINE_WIDTH, 0, LINE_WIDTH, mGridRect.height(), mLineColor);
}

void OrderGrid::resizeEvent(QResizeEvent *evt) {
    auto newh = evt->size().height();

    if (evt->oldSize().height() != newh) {
        mGridRect.setHeight(newh);
        mVisibleRows = mCellPainter.calculateRowsAvailable(newh);
        updatePatternRange();
    }
}

void OrderGrid::fontChanged() {
    mCellPainter.setFont(font());

    auto const cellWidth = mCellPainter.cellWidth();

    mGridRect.setX(2 * (SPACING + cellWidth) + LINE_WIDTH);
    mGridRect.setWidth(13 * cellWidth);

    // determine minimum width
    setMinimumWidth(mGridRect.right());

    mVisibleRows = mCellPainter.calculateRowsAvailable(height());
}

int OrderGrid::cursorX() const {
    int trackx = mCellPainter.cellWidth() * 3 * mModel.cursorTrack();
    if (!mHighNibble) {
        trackx += mCellPainter.cellWidth();
    }
    return mGridRect.x() + mCellPainter.cellWidth() + trackx;
}

void OrderGrid::setCursorPattern(int pattern) {
    if (mModel.isFollowing() && mModel.isPlaying()) {
        emit patternJump(pattern);
    } else {
        mModel.setCursorPattern(pattern);
    }
}

void OrderGrid::updatePatternRange() {
    auto totalRows = mModel.patterns();
    if (mVisibleRows > totalRows) {
        // the entire order can fit in the viewport
        mPatternStart = 0;
        mPatternEnd = totalRows;
    } else {
        // draw as much as we can fit with the cursor in the center
        mPatternStart = mModel.cursorPattern() - (mVisibleRows / 2);
        mPatternEnd = mPatternStart + mVisibleRows - 1;
        if (mPatternStart < 0) {
            mPatternEnd -= mPatternStart;
            mPatternStart = 0;
        }

        if (mPatternEnd > totalRows) {
            mPatternStart -= mPatternEnd - totalRows;
            mPatternEnd = totalRows;
        }

    }
}
