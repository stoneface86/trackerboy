
#include "core/clipboard/PatternClip.hpp"
#include "widgets/grid/PatternGrid.hpp"
#include "widgets/grid/layout.hpp"

#include "trackerboy/note.hpp"

#include <QApplication>
#include <QDrag>
#include <QFontDatabase>
#include <QMimeData>
#include <QPainter>
#include <QtDebug>
#include <QUndoCommand>

#include <algorithm>

using namespace PatternConstants;


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

PatternGrid::PatternGrid(PatternGridHeader &header, Document &document, QWidget *parent) :
    QWidget(parent),
    mDocument(document),
    mHeader(header),
    mPainter(font()),
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

    auto &songModel = document.songModel();

    connect(&songModel, &SongModel::rowsPerBeatChanged, this, &PatternGrid::setFirstHighlight);
    connect(&songModel, &SongModel::rowsPerMeasureChanged, this, &PatternGrid::setSecondHighlight);

    mPainter.setFirstHighlight(songModel.rowsPerBeat());
    mPainter.setSecondHighlight(songModel.rowsPerMeasure());

    auto &patternModel = document.patternModel();
    connect(&patternModel, &PatternModel::cursorChanged, this, &PatternGrid::updateCursor);
    // these changes require a full redraw
    connect(&patternModel, &PatternModel::invalidated, this, &PatternGrid::updateAll);
    connect(&patternModel, &PatternModel::selectionChanged, this, &PatternGrid::updateAll);
    // these we only need to redraw the cursor row
    connect(&patternModel, &PatternModel::recordingChanged, this, &PatternGrid::updateCursorRow);
    

    connect(&patternModel, &PatternModel::trackerCursorChanged, this, &PatternGrid::calculateTrackerRow);
    connect(&patternModel, &PatternModel::playingChanged, this, &PatternGrid::setPlaying);

}


void PatternGrid::setColors(ColorTable const& colors) {
    mPainter.setColors(colors);

    // update palette so the background is automatically drawn
    auto pal = palette();
    pal.setColor(backgroundRole(), colors[+Color::background]);
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
    Q_UNUSED(evt);

    QPainter painter(this);


    auto const h = height();
    auto const rowHeight = mPainter.cellHeight();
    unsigned const centerRow = mVisibleRows / 2;

    auto &patternModel = mDocument.patternModel();
    auto const cursor = patternModel.cursor();
    auto patternPrev = patternModel.previousPattern();
    auto patternCurr = patternModel.currentPattern();
    auto patternNext = patternModel.nextPattern();
    auto const rowsInPrevious = patternPrev ? (int)patternPrev->totalRows() : 0;
    auto const rowsInCurrent = (int)patternCurr.totalRows();
    auto const rowsInNext = patternNext ? (int)patternNext->totalRows() : 0;
    

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

        mPainter.drawBackground(painter, ypos, relativeRowIndex, rowsToDraw);

    }

    // [3] current row
    if (mTrackerRow) {
        mPainter.drawRowBackground(painter, PatternPainter::ROW_PLAYER, *mTrackerRow);
    }
    if (mEditorFocus) {
        mPainter.drawRowBackground(painter, patternModel.isRecording() ? PatternPainter::ROW_EDIT : PatternPainter::ROW_CURRENT, centerRow);
    }

    // [4] selection
    if (patternModel.hasSelection()) {
        auto selection = patternModel.selection();
        selection.translate(-cursor.row + (int)centerRow);
        mPainter.drawSelection(painter, selection);
    }

    // [5] cursor
    mPainter.drawCursor(painter, PatternCursor(centerRow, cursor.column, cursor.track));

    // [6] text
    {
        int rowYpos = 0;
        int rowsToDraw = mVisibleRows;
        int relativeRowIndex = cursor.row - centerRow;

        if (relativeRowIndex < 0) {
            // a negative row index means we draw from the previous pattern
            if (patternPrev) {
                uint16_t rowno;
                int rowIndexToPrev = relativeRowIndex + rowsInPrevious;
                if (rowIndexToPrev < 0) {
                    rowYpos += -rowIndexToPrev * rowHeight;
                    rowno = 0;
                } else {
                    rowno = (uint16_t)rowIndexToPrev;
                }
                painter.setOpacity(0.5);
                do {
                    rowYpos = mPainter.drawRow(painter, (*patternPrev)[rowno], rowno, rowYpos);
                    ++rowno;
                } while (rowno != rowsInPrevious);
                painter.setOpacity(1.0);
            } else {
                // previews disabled or we don't have a previous pattern, just skip these rows
                rowYpos += -relativeRowIndex * rowHeight;
            }
            rowsToDraw += relativeRowIndex;
            relativeRowIndex = 0;
        }

        // draw the current pattern
        uint16_t rowno = (uint16_t)relativeRowIndex;
        do {
            rowYpos = mPainter.drawRow(painter, patternCurr[rowno], rowno, rowYpos);
            ++rowno;
            --rowsToDraw;
        } while (rowno < rowsInCurrent && rowsToDraw > 0);

        if (rowsToDraw > 0 && patternNext) {
            // we have extra rows and a next pattern, draw it
            rowno = 0;
            painter.setOpacity(0.5);
            do {
                rowYpos = mPainter.drawRow(painter, (*patternNext)[rowno], rowno, rowYpos);
                ++rowno;
                --rowsToDraw;
            } while (rowno < rowsInNext && rowsToDraw > 0);
            painter.setOpacity(1.0);
        }
    }

    // [7] lines
    mPainter.drawLines(painter, h);

    if (mHasDrag) {
        auto selection = patternModel.selection();
        PatternAnchor pos = mDragPos;
        pos.row -= mDragRow;
        selection.moveTo(pos);
        selection.clamp(rowsInCurrent - 1);
        selection.translate((int)centerRow - cursor.row);
        auto rect = mPainter.selectionRectangle(selection);

        painter.setPen(Qt::white);
        painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
        painter.drawRect(rect);
    }

}

void PatternGrid::resizeEvent(QResizeEvent *evt) {

    auto oldSize = evt->oldSize();
    auto newSize = evt->size();

    if (newSize.height() != oldSize.height()) {
        mVisibleRows = getVisibleRows();
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

    auto &patternModel = mDocument.patternModel();
    auto pos = evt->pos();

    switch (mMouseOp) {

        case MouseOperation::nothing:
            break;
        case MouseOperation::selectingRows: {
            int row = mouseToRow(pos.y());
            if (rowIsValid(row)) {
                patternModel.selectRow(row);
            }
            break;
        }
        case MouseOperation::beginSelecting: {
            if ((pos - mMousePos).manhattanLength() > SELECTION_DEAD_ZONE) {
                patternModel.setSelection(mSelectionStart);
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
            mDocument.patternModel().setSelection(mSelectionEnd);
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
                mDocument.patternModel().moveSelection(cursor);

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

        auto &patternModel = mDocument.patternModel();
        
        
        mMousePos = evt->pos();

        if (mMousePos.x() < mPainter.rownoWidth()) {
            // select whole rows instead of cells
            patternModel.deselect();
            int row = mouseToRow(mMousePos.y());
            if (rowIsValid(row)) {
                mMouseOp = MouseOperation::selectingRows;
                patternModel.selectRow(row);
            }
        } else {
            // now convert the mouse coordinates to a coordinate on the grid
            auto cursor = mouseToCursor(mMousePos);
            if (cursor.isValid() && rowIsValid(cursor.row)) {
                mSelectionStart = cursor;

                if (patternModel.hasSelection()) {
                    // check if the mouse is within the selection rectangle
                    auto selection = patternModel.selection();
                    if (selection.contains(cursor)) {
                        mMouseOp = MouseOperation::dragging;
                        mDragRow = cursor.row - selection.iterator().rowStart();
                        return;
                    }
                }

                patternModel.deselect();
                mMouseOp = MouseOperation::beginSelecting;

            }
        }

    }
}

void PatternGrid::mouseReleaseEvent(QMouseEvent *evt) {

    if (evt->button() == Qt::LeftButton) {

        // if the user did not select anything, move the cursor to the starting coordinate
        if (mMouseOp == MouseOperation::beginSelecting || mMouseOp == MouseOperation::dragging) {
            auto &patternModel = mDocument.patternModel();
            patternModel.deselect();
            patternModel.setCursor(mSelectionStart);
        }

        mMouseOp = MouseOperation::nothing;
    }
}

// ======================================================= PRIVATE METHODS ===

void PatternGrid::updateCursor(PatternModel::CursorChangeFlags flags) {
    if (flags.testFlag(PatternModel::CursorRowChanged)) {
        updateAll();
    } else {
        updateCursorRow();
    }
}

void PatternGrid::updateCursorRow() {
    auto cellHeight = mPainter.cellHeight();
    QRect rect(mPainter.rownoWidth(), mVisibleRows / 2 * cellHeight, mPainter.trackWidth() * 4, cellHeight);
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

    mVisibleRows = getVisibleRows();

    auto const rownoWidth = mPainter.rownoWidth();
    auto const trackWidth = mPainter.trackWidth();
    mHeader.setWidths(rownoWidth, trackWidth);
}

int PatternGrid::mouseToRow(int const mouseY) {
    return mDocument.patternModel().cursorRow() + (mouseY / mPainter.cellHeight() - ((int)mVisibleRows / 2));
}

PatternCursor PatternGrid::mouseToCursor(QPoint const pos) {
    auto x = pos.x() - mPainter.rownoWidth();
    int col, track;
    if (x < 0) {
        col = -1;
        track = -1;
    } else {
        auto cell = x / mPainter.cellWidth();
        track = cell / TRACK_CELLS;
        cell %= TRACK_CELLS;
        col = TRACK_CELL_MAP[cell];

    }

    return {
        mouseToRow(pos.y()),
        col,
        track
    };
}

bool PatternGrid::rowIsValid(int row) {
    return row >= 0 && row < (int)mDocument.patternModel().currentPattern().totalRows();
}

void PatternGrid::clampCursor(PatternCursor &cursor) {
    cursor.row = std::clamp(cursor.row, 0, (int)mDocument.patternModel().currentPattern().totalRows() - 1);
    if (cursor.track >= PatternCursor::MAX_TRACKS) {
        cursor.track = PatternCursor::MAX_TRACKS - 1;
        cursor.column = PatternCursor::MAX_COLUMNS - 1;
    } else if (cursor.track < 0) {
        cursor.track = 0;
        cursor.column = 0;
    }
}

unsigned PatternGrid::getVisibleRows() {
    auto h = height();
    // integer division, rounding up
    return (h - 1) / mPainter.cellHeight() + 1;
}

void PatternGrid::calculateTrackerRow() {

    auto &patternModel = mDocument.patternModel();
    if (!patternModel.isFollowing() && patternModel.isPlaying()) {

        auto patternPrev = patternModel.previousPattern();
        auto patternNext = patternModel.nextPattern();
        auto const cursorRow = patternModel.cursorRow();
        int const centerRow = mVisibleRows / 2;
        auto const trackerRowCursor = patternModel.trackerCursorRow();
        auto const trackerPattern = patternModel.trackerCursorPattern();
        auto const currentPattern = mDocument.orderModel().currentPattern();
        int trackerRow = -1;
        if (currentPattern == trackerPattern) {
            trackerRow = trackerRowCursor - (cursorRow - centerRow);
        } else if (patternPrev && trackerPattern == currentPattern - 1) {
            trackerRow = (centerRow - cursorRow) - ((int)patternPrev->totalRows() - trackerRowCursor);
        } else if (patternNext && trackerPattern == currentPattern + 1) {
            trackerRow = (centerRow - cursorRow) + (int)patternNext->totalRows() + trackerRowCursor;
        }

        if (trackerRow > 0 && trackerRow < (int)mVisibleRows && trackerRow != centerRow) {
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
