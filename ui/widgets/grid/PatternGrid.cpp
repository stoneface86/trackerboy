
#include "widgets/grid/PatternGrid.hpp"
#include "widgets/grid/layout.hpp"

#include "trackerboy/note.hpp"

#include <QFontDatabase>
#include <QPainter>
#include <QtDebug>
#include <QUndoCommand>

#include <algorithm>

#define REQUIRE_DOCUMENT() if (Q_UNLIKELY(mDocument == nullptr)) return

using namespace PatternConstants;

static uint8_t replaceNibble(uint8_t value, uint8_t nibble, bool highNibble) {
    if (highNibble) {
        return (value & 0x0F) | (nibble << 4);
    } else {
        return (value & 0xF0) | (nibble);
    }
}


static void copyFromPattern(trackerboy::Pattern const& src, uint16_t rowno, int start, int count, char *dest) {
    int channel = start / sizeof(trackerboy::TrackRow);
    int offset = start % sizeof(trackerboy::TrackRow);
    while (count) {
        auto const& rowdata = src.getTrackRow(static_cast<trackerboy::ChType>(channel), rowno);
        auto toCopy = std::min(count, (int)sizeof(trackerboy::TrackRow) - offset);
        std::copy_n(reinterpret_cast<const char*>(&rowdata) + offset, toCopy, dest);
        dest += toCopy;
        offset = 0;
        ++channel;
        count -= toCopy;
    }
}

static void copyToPattern(const char* src, uint16_t rowno, int colFrom, int count, trackerboy::Pattern &dest) {
    int channel = colFrom / sizeof(trackerboy::TrackRow);
    int offset = colFrom % sizeof(trackerboy::TrackRow);
    while (count) {
        auto &rowdata = dest.getTrackRow(static_cast<trackerboy::ChType>(channel), rowno);
        auto toCopy = std::min(count, (int)sizeof(trackerboy::TrackRow) - offset);
        std::copy_n(src, toCopy, reinterpret_cast<char*>(&rowdata) + offset);
        src += toCopy;
        offset = 0;
        ++channel;
        count -= toCopy;
    }
}



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

//
// Pattern edit command. An edit to a pattern is a region of data that replaces
// a previous region of data. All edits to a pattern will use this class.
//
// class PatternEditCommand : public QUndoCommand {


// public:

//     PatternEditCommand(PatternGrid &grid, uint8_t pattern, QRect location) :
//         mGrid(grid),
//         mNewData(),
//         mOldData(),
//         mPattern(pattern),
//         mLocation(location),
//         mUpdatePatterns(false)
//     {
//         auto size = mLocation.width() * mLocation.height();
//         mNewData.fill('\0', size);
//         mOldData.fill('\0', size);

//         // pilfer the current data for undos
//         int const col = mLocation.left();
//         int const rowEnd = mLocation.bottom() + 1;
//         int const columnsToCopy = mLocation.width();
//         auto olddata = mOldData.data();
//         auto patternData = mGrid.mDocument->mod().song().getPattern(mPattern);

//         for (int row = mLocation.top(); row < rowEnd; ++row) {
//             copyFromPattern(patternData, (uint16_t)row, col, columnsToCopy, olddata);
//             olddata += columnsToCopy;
//         }
//     }

//     QByteArray oldData() const {
//         return mOldData;
//     }

//     QByteArray& newData() {
//         return mNewData;
//     }

//     void setUpdatePatterns(bool update) {
//         mUpdatePatterns = update;
//     }


//     virtual void redo() override {
//         setRegion(mNewData);
//     }

//     virtual void undo() override {
//         setRegion(mOldData);
//     }

// private:

//     void setRegion(QByteArray const& data) {
//         {
//             auto ctx = mGrid.mDocument->beginCommandEdit();

//             int const rowEnd = mLocation.bottom() + 1;
//             int const colsToCopy = mLocation.width();
//             int col = mLocation.left();

//             auto pattern = mGrid.mDocument->mod().song().getPattern(mPattern);

//             auto dataptr = data.data();
//             for (int row = mLocation.top(); row < rowEnd; ++row) {
//                 copyToPattern(dataptr, (uint16_t)row, col, colsToCopy, pattern);
//                 dataptr += colsToCopy;
//             }
//         }

//         if (mUpdatePatterns) {
//             mGrid.setPatterns(mPattern);
//         }
//         mGrid.update();

//     }

//     PatternGrid &mGrid;
//     QByteArray mNewData;
//     QByteArray mOldData;
//     uint8_t const mPattern;
//     QRect const mLocation; // location of the region being edited
//     bool mUpdatePatterns;
// };

//
// Command for editing a single column, ie setting a note, instrument, etc
//
// class PatternEditColumnCommand : public QUndoCommand {

// public:

//     enum DataColumn {
//         Note = offsetof(trackerboy::TrackRow, note),
//         Instrument = offsetof(trackerboy::TrackRow, instrumentId),
//         EffectType = offsetof(trackerboy::TrackRow, note)
//     };

//     PatternEditColumnCommand(
//         PatternGrid &grid,
//         uint8_t newData,
//         uint8_t oldData,
//         uint8_t pattern,
//         uint8_t track,
//         uint8_t row,
//         uint8_t offset
//     ) :
//         mGrid(grid),
//         mNewData(newData),
//         mOldData(oldData),
//         mPattern(pattern),
//         mRow(row),
//         mTrack(track),
//         mOffset(offset)
//     {
//     }

//     virtual void redo() override {
//         setData(mNewData);
//     }

//     virtual void undo() override {
//         setData(mOldData);
//     }

//     static PatternEditColumnCommand* setNoteCmd(PatternGrid &grid, std::optional<uint8_t> note) {
//         PatternEditColumnCommand *cmd = nullptr;

//         // get the rowdata
//         auto ch = grid.mCursorCol / TRACK_COLUMNS;
//         auto const& rowdata = grid.mDocument->mod().song().getRow(static_cast<trackerboy::ChType>(ch), grid.mCursorPattern, grid.mCursorRow);

//         auto old = rowdata.queryNote();
//         if (old != note) {
//             // new data != old data, create a command
//             cmd = new PatternEditColumnCommand(
//                 grid,
//                 trackerboy::TrackRow::convertColumn(note),
//                 trackerboy::TrackRow::convertColumn(old),
//                 grid.mCursorPattern,
//                 grid.mCursorRow,
//                 ch * sizeof(trackerboy::TrackRow) + offsetof(trackerboy::TrackRow, note)
//                 );
//             if (note) {
//                 cmd->setText(PatternGrid::tr("set note"));
//             } else {
//                 cmd->setText(PatternGrid::tr("clear note"));
//             }
//         }

//         return cmd;
    
//     }


// private:

//     void setData(uint8_t data) {

//         auto &rowdata = mGrid.mDocument->mod().song().getRow(
//             static_cast<trackerboy::ChType>(mTrack),
//             mPattern,
//             mRow
//         );

//         {
//             auto ctx = mGrid.mDocument->beginCommandEdit();
//             reinterpret_cast<uint8_t*>(&rowdata)[mOffset] = data;
//         }

//         if (mUpdatePatterns) {
//             mGrid.setPatterns(mGrid.mCursorPattern);
//         }
//         mGrid.update();
//     }

//     PatternGrid &mGrid;
//     uint8_t const mNewData;
//     uint8_t const mOldData;
//     uint8_t const mPattern;
//     uint8_t const mRow;
//     uint8_t const mTrack;
//     uint8_t const mOffset;
//     bool const mUpdatePatterns;

// };



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

static std::optional<trackerboy::EffectType> keyToEffectType(int const key) {
    switch (key) {
        case Qt::Key_B:
            return trackerboy::EffectType::patternGoto;
        case Qt::Key_C:
            return trackerboy::EffectType::patternHalt;
        case Qt::Key_D:
            return trackerboy::EffectType::patternSkip;
        case Qt::Key_F:
            return trackerboy::EffectType::setTempo;
        case Qt::Key_T:
            return trackerboy::EffectType::sfx;
        case Qt::Key_E:
            return trackerboy::EffectType::setEnvelope;
        case Qt::Key_V:
            return trackerboy::EffectType::setTimbre;
        case Qt::Key_I:
            return trackerboy::EffectType::setPanning;
        case Qt::Key_H:
            return trackerboy::EffectType::setSweep;
        case Qt::Key_S:
            return trackerboy::EffectType::delayedCut;
        case Qt::Key_G:
            return trackerboy::EffectType::delayedNote;
        case Qt::Key_L:
            return trackerboy::EffectType::lock;
        case Qt::Key_0:
            return trackerboy::EffectType::arpeggio;
        case Qt::Key_1:
            return trackerboy::EffectType::pitchUp;
        case Qt::Key_2:
            return trackerboy::EffectType::pitchDown;
        case Qt::Key_3:
            return trackerboy::EffectType::autoPortamento;
        case Qt::Key_4:
            return trackerboy::EffectType::vibrato;
        case Qt::Key_5:
            return trackerboy::EffectType::vibratoDelay;
        case Qt::Key_P:
            return trackerboy::EffectType::tuning;
        case Qt::Key_Q:
            return trackerboy::EffectType::noteSlideUp;
        case Qt::Key_R:
            return trackerboy::EffectType::noteSlideDown;
        default:
            return std::nullopt;
    }
}

static std::optional<uint8_t> keyToHex(int const key) {
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        return (uint8_t)(key - Qt::Key_0);
    } else if (key >= Qt::Key_A && key <= Qt::Key_F) {
        return (uint8_t)(key - Qt::Key_A + 0xA);
    } else {
        return std::nullopt;
    }
}


PatternGrid::PatternGrid(PatternGridHeader &header, QWidget *parent) :
    QWidget(parent),
    mDocument(nullptr),
    mHeader(header),
    mPainter(font()),
    mSelecting(false),
    mVisibleRows(0),
    mHasSelection(false),
    mSelectionStartX(0),
    mSelectionStartY(0),
    mSelectionEndX(0),
    mSelectionEndY(0)
{

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


bool PatternGrid::processKeyPress(PianoInput const& input, int const key) {
    bool validKey = false;

    #if 0
    PatternEditCommand *cmd = nullptr;

    auto const coltype = static_cast<ColumnType>(mCursorCol % TRACK_COLUMNS);
    auto const track = mCursorCol / TRACK_COLUMNS;

    switch (coltype) {
        case PatternConstants::COLUMN_NOTE: {
            auto note = input.keyToNote(key);
            if (note) {
                // TODO: preview the note
                //mPreviewKey = key;
                //emit previewNote(*note);
                // set it in the grid
                if (mEditMode) {
                    // set note
                    int dataCol = track * sizeof(trackerboy::TrackRow);
                    cmd = new PatternEditCommand(*this, (uint8_t)mCursorPattern, QRect(dataCol, mCursorRow, 1, 1));
                    cmd->newData()[0] = trackerboy::TrackRow::convertColumn(note);
                    cmd->setText(tr("edit note"));
                }

                validKey = true;
            }
            break;
        }
        case PatternConstants::COLUMN_EFFECT1_TYPE:
        case PatternConstants::COLUMN_EFFECT2_TYPE:
        case PatternConstants::COLUMN_EFFECT3_TYPE:
        {
            // check if the key pressed is a valid effect type
            auto effectType = keyToEffectType(key);
            if (effectType) {
                // shortcut for converting the column type to its corresponding effect number
                // assert that we can do this
                static_assert(PatternConstants::COLUMN_EFFECT1_TYPE / 3 - 1 == 0);
                static_assert(PatternConstants::COLUMN_EFFECT2_TYPE / 3 - 1 == 1);
                static_assert(PatternConstants::COLUMN_EFFECT3_TYPE / 3 - 1 == 2);

                if (mEditMode) {
                    // edit effect type
                    int effectNo = coltype / 3 - 1;
                    int dataCol = track * sizeof(trackerboy::TrackRow) + 2 + (effectNo * 2);
                    cmd = new PatternEditCommand(*this, (uint8_t)mCursorPattern, QRect(dataCol, mCursorRow, 1, 1));
                    cmd->newData()[0] = static_cast<char>(*effectType);
                    cmd->setUpdatePatterns(true);
                    cmd->setText(tr("Set effect type"));
                }

                validKey = true;
            }
        }
            break;
        case PatternConstants::COLUMN_INSTRUMENT_HIGH:
        case PatternConstants::COLUMN_INSTRUMENT_LOW: {
            auto hex = keyToHex(key);
            if (hex) {
                if (mEditMode) {
                    // get the current instrument value
                    uint8_t instrument = mPatternCurr->operator[](
                        (uint16_t)mCursorRow
                    )[track].queryInstrument().value_or((uint8_t)0);
                    instrument = replaceNibble(instrument, *hex, coltype == PatternConstants::COLUMN_INSTRUMENT_HIGH);
                    
                    if (instrument >= trackerboy::MAX_INSTRUMENTS) {
                        // ignore this key if we go over the maximum possible instrument id
                        break;
                    }

                    int dataCol = (track * sizeof(trackerboy::TrackRow)) + 1;
                    cmd = new PatternEditCommand(*this, (uint8_t)mCursorPattern, QRect(dataCol, mCursorRow, 1, 1));
                    cmd->newData()[0] = (char)trackerboy::TrackRow::convertColumn(instrument);
                    cmd->setText(tr("set instrument"));
                
                }
                validKey = true;
            }
            break;
        }
        case PatternConstants::COLUMN_EFFECT1_ARG_HIGH:
        case PatternConstants::COLUMN_EFFECT1_ARG_LOW:
        case PatternConstants::COLUMN_EFFECT2_ARG_HIGH:
        case PatternConstants::COLUMN_EFFECT2_ARG_LOW:
        case PatternConstants::COLUMN_EFFECT3_ARG_HIGH:
        case PatternConstants::COLUMN_EFFECT3_ARG_LOW:
        {


            // check if the key pressed is a hex number
            auto hex = keyToHex(key);
            if (hex) {
                if (mEditMode) {
                    // edit instrument/effect arg
                    int coltypeAdj = coltype - 3;
                    int effectNo = coltypeAdj / 3;
                    int nibbleIndex = coltypeAdj % 3;
                    uint8_t arg = mPatternCurr->operator[](
                        (uint16_t)mCursorRow
                    )[track].effects[effectNo].param;
                    arg = replaceNibble(arg, *hex, nibbleIndex == 1);

                    int dataCol = track * sizeof(trackerboy::TrackRow) + 3 + (effectNo * 2);
                    cmd = new PatternEditCommand(*this, (uint8_t)mCursorPattern, QRect(dataCol, mCursorRow, 1, 1));
                    cmd->newData()[0] = (char)arg;
                    cmd->setText(tr("set effect parameter"));
                }

                validKey = true;
            }
        }
        break;
    }


    if (cmd) {
        mDocument->undoStack().push(cmd);
    }

    #endif

    return validKey;
}

void PatternGrid::processKeyRelease(int const key) {
    Q_UNUSED(key)
    // if (key == mPreviewKey) {
    //     emit stopNotePreview();
    // }
}

void PatternGrid::setDocument(ModuleDocument *doc) {
    if (mDocument) {
        mDocument->songModel().disconnect(this);
        mDocument->patternModel().disconnect(this);
    }


    mDocument = doc;

    if (doc) {

        auto &songModel = doc->songModel();
        // TODO: PatternModel needs to handle this
        // connect(&songModel, &SongModel::patternSizeChanged, this,
        //     [this](int rows) {
        //         if (mCursorRow >= rows) {
        //             mCursorRow = rows - 1;
        //             emit cursorRowChanged(mCursorRow);
        //         }
        //         setPatterns(mCursorPattern);
        //         setPatternRect();
        //         update();
        //     });

        connect(&songModel, &SongModel::rowsPerBeatChanged, this,
            [this](int rpb) {
                mPainter.setFirstHighlight(rpb);
                update();
            });
        connect(&songModel, &SongModel::rowsPerMeasureChanged, this,
            [this](int rpm) {
                mPainter.setSecondHighlight(rpm);
                update();
            });

        mPainter.setFirstHighlight(songModel.rowsPerBeat());
        mPainter.setSecondHighlight(songModel.rowsPerMeasure());

        calculateTrackerRow();

        auto &patternModel = doc->patternModel();
        // these changes require a full redraw
        connect(&patternModel, &PatternModel::cursorRowChanged, this, &PatternGrid::updateAll);
        connect(&patternModel, &PatternModel::patternsChanged, this, &PatternGrid::updateAll);
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
    mPainter.drawRowBackground(painter, patternModel.isRecording() ? PatternPainter::ROW_EDIT : PatternPainter::ROW_CURRENT, centerRow);

    // [4] selection
    // TODO!

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

    // for debugging purposes
    //painter.setPen(Qt::cyan);
    //painter.drawRect(mPatternRect);
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
    Q_UNUSED(evt);
    //int mx = evt->x();
    //int my = evt->y();
    if (mSelecting) {
        
        unsigned row, column;
        getCursorFromMouse(evt->x(), evt->y(), row, column);
        update();
        if (!mHasSelection || row != mSelectionEndY || column != mSelectionEndX) {
            mHasSelection = true;
            mSelectionEndX = column;
            mSelectionEndY = row;
            update();
        }
    }

}

void PatternGrid::mousePressEvent(QMouseEvent *evt) {
    if (evt->button() == Qt::LeftButton) {

        mSelecting = true;
        getCursorFromMouse(evt->x(), evt->y(), mSelectionStartY, mSelectionStartX);
        mSelectionEndX = mSelectionStartX;
        mSelectionEndY = mSelectionEndY;
    }
}

void PatternGrid::mouseReleaseEvent(QMouseEvent *evt) {

    if (evt->button() == Qt::LeftButton) {
        mSelecting = false;
        // int mx = evt->x(), my = evt->y();

        // if (mPatternRect.contains(mx, my)) {
        //     unsigned row, column;
        //     mx -= mPainter.rownoWidth();
        //     getCursorFromMouse(mx, my, row, column);

        //     setCursorRow(row);
        //     setCursorColumn(column);
        // }
        
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

void PatternGrid::fontChanged() {

    mVisibleRows = getVisibleRows();

    auto const rownoWidth = mPainter.rownoWidth();
    auto const trackWidth = mPainter.trackWidth();
    mHeader.setWidths(rownoWidth, trackWidth);
}

void PatternGrid::getCursorFromMouse(int x, int y, unsigned &outRow, unsigned &outCol) {
    int cell = x / mPainter.cellWidth();
    int track = cell / TRACK_CELLS;
    cell = cell % TRACK_CELLS;
    outCol = (track * TRACK_COLUMNS) + TRACK_CELL_MAP[cell];
    outRow = mDocument->patternModel().cursorRow() + (y / mPainter.cellHeight() - (mVisibleRows / 2));

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