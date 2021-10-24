
#include "core/clipboard/PatternClip.hpp"
#include "widgets/grid/PatternGrid.hpp"

#include "trackerboy/note.hpp"

#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QPainter>
#include <QtDebug>

#include <algorithm>


// Philisophy note
// should undo'ing modify the cursor? Is the cursor considered document state?
// For example, if I enter a note with edit step set to 4, the cursor will be
// moved 4 rows down after setting the note. If I undo this edit, should the
// cursor go back to where it was or should it remain where it is?
// Compared to other trackers, Famitracker does the former and OpenMPT does the
// latter.
//
// The problem that arises from this is that if I move the cursor and then undo,
// that change I made to the cursor is lost. Should it be? Or should it not?
// We'll leave the cursor unchanged on undo for now (since that's easiest to 
// program ;) )


// A cell is a single character on the grid, columns are 1 or more cells. A
// cell with a space is used as spacing, and the adjacent column is selected if
// the user clicks on it.
//
// The first 4 cells of the grid are reserved for the row numbers, and are not selectable

PatternGrid::PatternGrid(
    PatternGridHeader &header,
    PatternModel &model,
    QWidget *parent
) :
    QWidget(parent),
    mHeader(header),
    mModel(model),
    mPainter(font()),
    mShowShadow(true),
    mSelecting(false),
    mVisibleRows(0),
    mEditorFocus(false),
    mMousePos(),
    mSelectionStart(),
    mSelectionEnd(),
    mHasDrag(false),
    mDragPos(),
    mDragRow(0),
    mMouseOp(MouseOperation::nothing)
{
    setAcceptDrops(true);
    setAutoFillBackground(true);

    // first time initialization
    fontChanged();

    connect(&model, &PatternModel::cursorChanged, this, &PatternGrid::updateCursor);
    // these changes require a full redraw
    connect(&model, &PatternModel::invalidated, this, &PatternGrid::updateAll);
    connect(&model, &PatternModel::selectionChanged, this, &PatternGrid::updateAll);
    // these we only need to redraw the cursor row
    connect(&model, &PatternModel::recordingChanged, this, &PatternGrid::updateCursorRow);
    

    connect(&model, &PatternModel::trackerCursorChanged, this, &PatternGrid::calculateTrackerRow);
    connect(&model, &PatternModel::playingChanged, this, &PatternGrid::setPlaying);

    connect(&model, &PatternModel::effectsVisibleChanged, this,
        [this]() {
            // update the layout
            auto counts = mModel.effectsVisible();
            for (size_t i = 0; i < counts.size(); ++i) {
                mLayout.setEffectsVisible((int)i, counts[i]);
            }
            // redraw everything
            update();
            mHeader.update();

        });


    header.setPatternLayout(&mLayout);
}

PatternGrid::~PatternGrid() {
    //mHeader.setPatternLayout(nullptr);
}

void PatternGrid::setColors(Palette const& colors) {
    mPainter.setColors(colors);

    // update palette so the background is automatically drawn
    auto pal = palette();
    pal.setColor(backgroundRole(), colors[Palette::ColorBackground]);
    setPalette(pal);

    // new colors, redraw everything
    update();
}

void PatternGrid::setShowFlats(bool showFlats) {
    if (showFlats != mPainter.flats()) {
        mPainter.setFlats(showFlats);
        update();
    }
}

void PatternGrid::setShowShadow(bool shadow) {
    if (shadow != mShowShadow) {
        mShowShadow = shadow;
        update();
    }
}

void PatternGrid::setEditorFocus(bool focus) {
    if (mEditorFocus != focus) {
        mEditorFocus = focus;
        updateCursorRow();
    }
}



// ================================================================ EVENTS ===

void PatternGrid::changeEvent(QEvent *evt) {
    if (evt->type() == QEvent::FontChange) {
        mPainter.setFont(font());
        fontChanged();
    }
}

void PatternGrid::dragEnterEvent(QDragEnterEvent *evt) {
    if (evt->source() == this && evt->proposedAction() == Qt::MoveAction) {
       // only accepts drags from this application (internal moves) 
        
        evt->acceptProposedAction();
        mHasDrag = true;
        update();
    }
}

void PatternGrid::dragLeaveEvent(QDragLeaveEvent *evt) {
    Q_UNUSED(evt)
    mHasDrag = false;
    update();
}

void PatternGrid::dragMoveEvent(QDragMoveEvent *evt) {

    auto pos = evt->pos();

    auto cursor = mouseToCursor(pos);
    cursor.column = std::clamp(cursor.column, 0, PatternCursor::MAX_COLUMNS - 1);
    cursor.track = std::clamp(cursor.track, 0, PatternCursor::MAX_TRACKS - 1);
    //cursor.column = PatternSelection::selectColumn(cursor.column);
    if (cursor != mDragPos) {
        mDragPos = cursor;
        
        update();
    }


}

void PatternGrid::dropEvent(QDropEvent *evt) {
    if (evt->proposedAction() == Qt::MoveAction) {
        evt->acceptProposedAction();
    }
}

void PatternGrid::paintEvent(QPaintEvent *evt) {
    Q_UNUSED(evt)

    QPainter painter(this);


    auto const h = height();
    auto const rowHeight = mPainter.cellHeight();
    auto const centerRow = mVisibleRows / 2;

    auto const cursor = mModel.cursor();
    auto patternPrev = mModel.previousPattern();
    auto patternCurr = mModel.currentPattern();
    auto patternNext = mModel.nextPattern();
    auto const rowsInPrevious = patternPrev ? patternPrev->totalRows() : 0;
    auto const rowsInCurrent = patternCurr.totalRows();
    auto const rowsInNext = patternNext ? patternNext->totalRows() : 0;

    // Z-order (from back to front)
    // 1. window background (drawn by qwidget)
    // 2. row background
    // 3. current row
    // 4. selection
    // 5. cursor
    // 6. pattern text (mDisplay)
    // 7. lines

    // [2] row background
    {
        int rowsToDraw = mVisibleRows;
        int relativeRowIndex = cursor.row - centerRow;
        int ypos = 0;
        int rowMin = -rowsInPrevious;
        int rowMax = rowsInCurrent + rowsInNext;

        if (relativeRowIndex < rowMin) {
            auto rowsToSkip = rowMin - relativeRowIndex;
            ypos += rowHeight * rowsToSkip;
            rowsToDraw -= rowsToSkip;
            relativeRowIndex = rowMin;
        }

        rowsToDraw = std::min(rowsToDraw, rowMax - relativeRowIndex);

        mPainter.drawBackground(painter, mLayout, ypos, relativeRowIndex, rowsToDraw);

    }

    // [3] current row
    if (mTrackerRow) {
        mPainter.drawRowBackground(painter, mLayout, PatternPainter::RowPlayer, *mTrackerRow);
    }
    if (mEditorFocus) {
        mPainter.drawRowBackground(painter, mLayout, mModel.isRecording() ? PatternPainter::RowEdit : PatternPainter::RowCurrent, centerRow);
    }

    // [4] selection
    if (mModel.hasSelection()) {
        auto selection = mModel.selection();
        selection.translate(-cursor.row + centerRow);
        auto rect = mLayout.selectionRectangle(selection);
        mPainter.drawSelection(painter, rect);
    }

    // [5] cursor
    mPainter.drawCursor(painter, mLayout, PatternCursor(centerRow, cursor.column, cursor.track));

    // [6] text
    {
        int rowYpos = 0;
        int rowsToDraw = mVisibleRows;
        int relativeRowIndex = cursor.row - centerRow;

        if (relativeRowIndex < 0) {
            // a negative row index means we draw from the previous pattern
            if (patternPrev) {
                int rowno;
                int rowIndexToPrev = relativeRowIndex + rowsInPrevious;
                if (rowIndexToPrev < 0) {
                    // clamp
                    rowYpos += -rowIndexToPrev * rowHeight;
                    rowno = 0;
                } else {
                    rowno = rowIndexToPrev;
                }
                painter.setOpacity(0.5);
                rowYpos = mPainter.drawPattern(painter, mLayout, *patternPrev, rowno, rowsInPrevious - 1, rowYpos);
                painter.setOpacity(1.0);
            } else {
                // previews disabled or we don't have a previous pattern, just skip these rows
                rowYpos += -relativeRowIndex * rowHeight;
            }

            rowsToDraw += relativeRowIndex;
            relativeRowIndex = 0;
        }

        // draw the current pattern
        int rowEnd = relativeRowIndex + rowsToDraw;
        if (rowEnd >= rowsInCurrent) {
            rowEnd = rowsInCurrent - 1;
        }
        rowYpos = mPainter.drawPattern(painter, mLayout, patternCurr, relativeRowIndex, rowEnd, rowYpos);
        rowsToDraw -= rowEnd - relativeRowIndex;

        if (rowsToDraw > 0 && patternNext) {
            // we have extra rows and a next pattern, draw it
            if (rowsToDraw > rowsInNext) {
                rowsToDraw = rowsInNext;
            }
            painter.setOpacity(0.5);
            mPainter.drawPattern(painter, mLayout, *patternNext, 0, rowsToDraw - 1, rowYpos);
            painter.setOpacity(1.0);
        }
    }

    // [7] lines
    mPainter.drawLines(painter, mLayout, h);

    if (mHasDrag) {
        auto selection = mModel.selection();
        PatternAnchor pos = mDragPos;
        pos.row -= mDragRow;
        selection.moveTo(pos);
        selection.clamp(rowsInCurrent - 1);
        selection.translate(centerRow - cursor.row);
        auto rect = mLayout.selectionRectangle(selection);

        painter.setPen(Qt::white);
        painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
        painter.drawRect(rect);
    }


    // drop shadow from header
    if (mShowShadow) {
        auto const w = width();
        painter.fillRect(0, 0, w, 1, QColor(0, 0, 0, 180));
        painter.fillRect(0, 1, w, 1, QColor(0, 0, 0, 120));
        painter.fillRect(0, 2, w, 1, QColor(0, 0, 0, 60));
    }
}

void PatternGrid::resizeEvent(QResizeEvent *evt) {

    auto oldSize = evt->oldSize();
    auto newSize = evt->size();

    if (newSize.height() != oldSize.height()) {
        mVisibleRows = mPainter.calculateRowsAvailable(newSize.height());
        calculateTrackerRow();
        
    }

}

void PatternGrid::leaveEvent(QEvent *evt) {
    Q_UNUSED(evt);
}

void PatternGrid::mouseMoveEvent(QMouseEvent *evt) {
    if (!evt->buttons().testFlag(Qt::LeftButton)) {
        return;
    }

    auto pos = evt->pos();

    switch (mMouseOp) {

        case MouseOperation::nothing:
            break;
        case MouseOperation::selectingRows: {
            int row = mouseToRow(pos.y());
            if (rowIsValid(row)) {
                mModel.selectRow(row);
            }
            break;
        }
        case MouseOperation::beginSelecting: {
            if ((pos - mMousePos).manhattanLength() > SELECTION_DEAD_ZONE) {
                mModel.setSelection(mSelectionStart);
                mMouseOp = MouseOperation::selecting;
            } else {
                break;
            }
        }
        [[fallthrough]];
        case MouseOperation::selecting:
            // update end coordinate for selection
            mSelectionEnd = mouseToCursor(pos);
            clampCursor(mSelectionEnd);
            mModel.setSelection(mSelectionEnd);
            break;
        case MouseOperation::dragging: {
            if ((pos - mMousePos).manhattanLength() < QApplication::startDragDistance()) {
                break;
            }

            QDrag *drag = new QDrag(this);
            // empty mimeData (dragging results in an internal move)
            auto mimeData = new QMimeData;
            drag->setMimeData(mimeData);

            drag->exec();
            if (mHasDrag) {
                // do the move
                auto cursor = mDragPos;
                cursor.row -= mDragRow;
                mModel.moveSelection(cursor);

                mHasDrag = false;
                update();
            }
            mMouseOp = MouseOperation::nothing;
            
            break;
        }

    }

}

void PatternGrid::mousePressEvent(QMouseEvent *evt) {
    if (evt->button() == Qt::LeftButton) {

        mMousePos = evt->pos();

        if (mMousePos.x() < mLayout.patternStart()) {
            // select whole rows instead of cells
            mModel.deselect();
            int row = mouseToRow(mMousePos.y());
            if (rowIsValid(row)) {
                mMouseOp = MouseOperation::selectingRows;
                mModel.selectRow(row);
            }
        } else {
            // now convert the mouse coordinates to a coordinate on the grid
            auto cursor = mouseToCursor(mMousePos);
            if (cursor.isValid() && rowIsValid(cursor.row)) {
                mSelectionStart = cursor;

                if (mModel.hasSelection()) {
                    // check if the mouse is within the selection rectangle
                    auto selection = mModel.selection();
                    if (selection.contains(cursor)) {
                        mMouseOp = MouseOperation::dragging;
                        mDragRow = cursor.row - selection.iterator().rowStart();
                        return;
                    }
                }

                mModel.deselect();
                mMouseOp = MouseOperation::beginSelecting;

            }
        }

    }
}

void PatternGrid::mouseReleaseEvent(QMouseEvent *evt) {

    if (evt->button() == Qt::LeftButton) {

        // if the user did not select anything, move the cursor to the starting coordinate
        if (mMouseOp == MouseOperation::beginSelecting || mMouseOp == MouseOperation::dragging) {
            mModel.deselect();
            mModel.setCursor(mSelectionStart);
        }

        mMouseOp = MouseOperation::nothing;
    }
}

// ======================================================= PRIVATE METHODS ===

void PatternGrid::updateCursor(PatternModel::CursorChangeFlags flags) {

    if (flags | (PatternModel::CursorRowChanged | PatternModel::CursorTrackChanged)) {
        updateAll();
    } else {
        updateCursorRow();
    }
}

void PatternGrid::updateCursorRow() {
    auto cellHeight = mPainter.cellHeight();
    QRect rect(mLayout.patternStart(), mVisibleRows / 2 * cellHeight, mLayout.rowWidth(), cellHeight);
    update(rect);
}

void PatternGrid::updateAll() {
    calculateTrackerRow();
    update();
}

void PatternGrid::setPlaying(bool playing) {
    if (!playing && mTrackerRow) {
        // this just hides the player row if it was set
        mTrackerRow.reset();
        update();
    }
}

void PatternGrid::setFirstHighlight(int highlight) {
    mPainter.setFirstHighlight(highlight);
    update();
}

void PatternGrid::setSecondHighlight(int highlight) {
    mPainter.setSecondHighlight(highlight);
    update();
}

void PatternGrid::fontChanged() {

    mVisibleRows = mPainter.calculateRowsAvailable(height());
    mLayout.setCellSize(mPainter.cellWidth(), mPainter.cellHeight());
    //auto const rownoWidth = mPainter.rownoWidth();
    //auto const trackWidth = mPainter.trackWidth();
    //mHeader.setWidths(rownoWidth, trackWidth);
}

int PatternGrid::mouseToRow(int const mouseY) {
    return mModel.cursorRow() + (mouseY / mPainter.cellHeight() - (mVisibleRows / 2));
}

PatternCursor PatternGrid::mouseToCursor(QPoint const pos) {
    auto cursor = mLayout.mouseToCursor(pos);
    cursor.row = mouseToRow(pos.y());
    return cursor;
}

bool PatternGrid::rowIsValid(int row) {
    return row >= 0 && row < mModel.currentPattern().totalRows();
}

void PatternGrid::clampCursor(PatternCursor &cursor) {
    cursor.row = std::clamp(cursor.row, 0, mModel.currentPattern().totalRows() - 1);
    if (cursor.track >= PatternCursor::MAX_TRACKS) {
        cursor.track = PatternCursor::MAX_TRACKS - 1;
        cursor.column = PatternCursor::MAX_COLUMNS - 1;
    } else if (cursor.track < 0) {
        cursor.track = 0;
        cursor.column = 0;
    }
}

void PatternGrid::calculateTrackerRow() {
    
    if (!mModel.isFollowing() && mModel.isPlaying()) {

        auto patternPrev = mModel.previousPattern();
        auto patternNext = mModel.nextPattern();
        auto const cursorRow = mModel.cursorRow();
        int const centerRow = mVisibleRows / 2;
        auto const trackerRowCursor = mModel.trackerCursorRow();
        auto const trackerPattern = mModel.trackerCursorPattern();
        auto const currentPattern = mModel.cursorPattern();
        int trackerRow = -1;
        if (currentPattern == trackerPattern) {
            trackerRow = trackerRowCursor - (cursorRow - centerRow);
        } else if (patternPrev && trackerPattern == currentPattern - 1) {
            trackerRow = (centerRow - cursorRow) - (patternPrev->totalRows() - trackerRowCursor);
        } else if (patternNext && trackerPattern == currentPattern + 1) {
            trackerRow = (centerRow - cursorRow) + patternNext->totalRows() + trackerRowCursor;
        }

        if (trackerRow > 0 && trackerRow < mVisibleRows && trackerRow != centerRow) {
            mTrackerRow = trackerRow;
            update();
            return;
        }
    } 

    if (mTrackerRow) {
        mTrackerRow.reset();
        update();
    }

}
