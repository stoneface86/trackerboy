
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

#define REQUIRE_DOCUMENT() if (Q_UNLIKELY(mDocument == nullptr)) return

using namespace PatternConstants;

//
// MIME type for drag n drop. The data that gets passed to QMimeData is a copy
// of PatternChunk's internal buffer (converted to QByteArray)
//
constexpr auto MIME_TYPE = "application/trackerboy";

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

PatternGrid::PatternGrid(PatternGridHeader &header, QWidget *parent) :
    QWidget(parent),
    mDocument(nullptr),
    mHeader(header),
    mPainter(font()),
    mSelecting(false),
    mVisibleRows(0),
    mEditorFocus(false),
    mMousePos(),
    mSelectionStartCoords(),
    mSelectionEndCoords(),
    mMouseOp(MouseOperation::nothing)
{
    setAcceptDrops(true);
    setAutoFillBackground(true);

    // first time initialization
    fontChanged();

}


void PatternGrid::setColors(ColorTable const& colors) {
    mPainter.setColors(colors);

    // update palette so the background is automatically drawn
    auto pal = palette();
    pal.setColor(QPalette::Window, colors[+Color::background]);
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

void PatternGrid::setDocument(ModuleDocument *doc) {
    if (mDocument) {
        mDocument->songModel().disconnect(this);
        mDocument->patternModel().disconnect(this);
    }


    mDocument = doc;

    if (doc) {

        auto &songModel = doc->songModel();

        connect(&songModel, &SongModel::rowsPerBeatChanged, this, &PatternGrid::setFirstHighlight);
        connect(&songModel, &SongModel::rowsPerMeasureChanged, this, &PatternGrid::setSecondHighlight);

        mPainter.setFirstHighlight(songModel.rowsPerBeat());
        mPainter.setSecondHighlight(songModel.rowsPerMeasure());

        calculateTrackerRow();

        auto &patternModel = doc->patternModel();
        // these changes require a full redraw
        connect(&patternModel, &PatternModel::cursorRowChanged, this, &PatternGrid::updateAll);
        connect(&patternModel, &PatternModel::invalidated, this, &PatternGrid::updateAll);
        connect(&patternModel, &PatternModel::selectionChanged, this, &PatternGrid::updateAll);
        // these we only need to redraw the cursor row
        connect(&patternModel, &PatternModel::cursorColumnChanged, this, &PatternGrid::updateCursorRow);
        connect(&patternModel, &PatternModel::recordingChanged, this, &PatternGrid::updateCursorRow);
        

        connect(&patternModel, &PatternModel::trackerCursorChanged, this, &PatternGrid::calculateTrackerRow);
        connect(&patternModel, &PatternModel::playingChanged, this, &PatternGrid::setPlaying);

    }

    update();
}


// ================================================================ EVENTS ===

void PatternGrid::changeEvent(QEvent *evt) {
    if (evt->type() == QEvent::FontChange) {
        mPainter.setFont(font());
        fontChanged();
    }
}

void PatternGrid::dragEnterEvent(QDragEnterEvent *evt) {
    if (evt->mimeData()->hasFormat(MIME_TYPE)) {
        evt->acceptProposedAction();
    }
}

void PatternGrid::dragMoveEvent(QDragMoveEvent *evt) {

}

void PatternGrid::dropEvent(QDropEvent *evt) {
    if (evt->source() == this && evt->proposedAction() == Qt::MoveAction) {
        evt->acceptProposedAction();
    }
}

void PatternGrid::paintEvent(QPaintEvent *evt) {
    Q_UNUSED(evt);

    REQUIRE_DOCUMENT();

    QPainter painter(this);


    auto const h = height();
    auto const rownoWidth = mPainter.rownoWidth();
    auto const rowHeight = mPainter.cellHeight();
    auto const trackWidth = mPainter.trackWidth();
    unsigned const centerRow = mVisibleRows / 2;

    auto &patternModel = mDocument->patternModel();
    auto const cursorRow = patternModel.cursorRow();
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
        int relativeRowIndex = cursorRow - centerRow;
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
        selection.translate(QPoint(0, -cursorRow + (int)centerRow));
        mPainter.drawSelection(painter, selection);
    }

    // [5] cursor
    mPainter.drawCursor(painter, centerRow, patternModel.cursorColumn());

    // [6] text
    {
        int rowYpos = 0;
        int rowsToDraw = mVisibleRows;
        int relativeRowIndex = cursorRow - centerRow;

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

    if (mMouseOp == MouseOperation::dragging) {
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setPen(Qt::white);
        painter.drawRect(mDragDest);
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

    auto &patternModel = mDocument->patternModel();
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
                patternModel.setSelection(mSelectionStartCoords);
                mMouseOp = MouseOperation::selecting;
            } else {
                break;
            }
        }
        [[fallthrough]];
        case MouseOperation::selecting: {
            // update end coordinate for selection
            QPoint coords(
                mouseToColumn(pos.x()),
                mouseToRow(pos.y())
            );
            mSelectionEndCoords = coordsClamped(coords);
            mDocument->patternModel().setSelection(mSelectionEndCoords);
            break;
        }
        case MouseOperation::dragging: {
            if ((pos - mMousePos).manhattanLength() < QApplication::startDragDistance()) {
                break;
            }

            QDrag *drag = new QDrag(this);
            // put the pattern data into QMimeData
            auto mimeData = new QMimeData;
            mimeData->setData(MIME_TYPE, QByteArray());
            drag->setMimeData(mimeData);

            auto dropAction = drag->exec();
            
            
            break;
        }

    }

}

void PatternGrid::mousePressEvent(QMouseEvent *evt) {
    if (evt->button() == Qt::LeftButton) {

        auto &patternModel = mDocument->patternModel();
        
        
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

            if (patternModel.hasSelection()) {
                // check if the mouse is within the selection rectangle
                auto selection = patternModel.selection();
                selection.translate(QPoint(0, -patternModel.cursorRow() + (int)(mVisibleRows / 2)));
                selection = mPainter.selectionRectangle(selection);
                if (selection.contains(mMousePos)) {
                    mMouseOp = MouseOperation::dragging;
                    mDragDest = selection;
                    return;
                }
            }

            patternModel.deselect();

            // now convert the mouse coordinates to a coordinate on the grid
            int column = mouseToColumn(mMousePos.x());
            if (column >= 0 && column < PatternModel::SELECTS) {
                int row = mouseToRow(mMousePos.y());
                if (rowIsValid(row)) {
                    mSelectionStartCoords = QPoint(column, row);
                    mMouseOp = MouseOperation::beginSelecting;
                }
            }
        }

    }
}

void PatternGrid::mouseReleaseEvent(QMouseEvent *evt) {

    if (evt->button() == Qt::LeftButton) {

        // if the user did not select anything, move the cursor to the starting coordinate
        if (mMouseOp == MouseOperation::beginSelecting || mMouseOp == MouseOperation::dragging) {
            auto &patternModel = mDocument->patternModel();
            patternModel.deselect();
            patternModel.setCursorColumn(mouseToCell(mMousePos.x()));
            patternModel.setCursorRow(mouseToRow(mMousePos.y()));
        }

        mMouseOp = MouseOperation::nothing;
    }
}

// ======================================================= PRIVATE METHODS ===

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
    return mDocument->patternModel().cursorRow() + (mouseY / mPainter.cellHeight() - ((int)mVisibleRows / 2));
}

int PatternGrid::mouseToCell(int const mouseX) {
    int cell = (mouseX - mPainter.rownoWidth()) / mPainter.cellWidth();
    if (cell < 0) {
        return -1;
    }
    int track = cell / TRACK_CELLS;
    cell = cell % TRACK_CELLS;
    return (track * TRACK_COLUMNS) + TRACK_CELL_MAP[cell];
}

int PatternGrid::mouseToColumn(int const mouseX) {
    int cell = (mouseX - mPainter.rownoWidth()) / mPainter.cellWidth();
    if (cell < 0) {
        return -1;
    }
    int track = cell / TRACK_CELLS;
    cell = cell % TRACK_CELLS;

    return (track * PatternModel::SELECTS_PER_TRACK) + TRACK_CELL_TO_DATA_COLUMN[cell];
}

// bool PatternGrid::coordsValid(QPoint const coords) {
//     return coords.x() >= 0 &&
//            coords.x() < (PatternModel::COLUMNS_PER_TRACK * 4) &&
//            coords.y() >= 0 &&
//            coords.y() < (int)mDocument->patternModel().currentPattern().totalRows();
// }

bool PatternGrid::rowIsValid(int row) {
    return row >= 0 && row < (int)mDocument->patternModel().currentPattern().totalRows();
}

QPoint PatternGrid::coordsClamped(QPoint const coords) {
    return {
        std::clamp(coords.x(), 0, PatternModel::SELECTS - 1),
        std::clamp(coords.y(), 0, (int)mDocument->patternModel().currentPattern().totalRows() - 1)
    };
}

unsigned PatternGrid::getVisibleRows() {
    auto h = height();
    // integer division, rounding up
    return (h - 1) / mPainter.cellHeight() + 1;
}

void PatternGrid::calculateTrackerRow() {
    REQUIRE_DOCUMENT();

    auto &patternModel = mDocument->patternModel();
    if (!patternModel.isFollowing() && patternModel.isPlaying()) {

        auto patternPrev = patternModel.previousPattern();
        auto patternNext = patternModel.nextPattern();
        auto const cursorRow = patternModel.cursorRow();
        int const centerRow = mVisibleRows / 2;
        auto const trackerRowCursor = patternModel.trackerCursorRow();
        auto const trackerPattern = patternModel.trackerCursorPattern();
        auto const currentPattern = mDocument->orderModel().currentPattern();
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