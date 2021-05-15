
#include "widgets/grid/PatternGrid.hpp"
#include "widgets/grid/layout.hpp"

#include "trackerboy/note.hpp"

#include <QFontDatabase>
#include <QPainter>
#include <QtDebug>

#include <algorithm>

#define REQUIRE_DOCUMENT() if (Q_UNLIKELY(mDocument == nullptr)) return

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

static std::optional<int> keyToHex(int const key) {
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        return key - Qt::Key_0;
    } else if (key >= Qt::Key_A && key <= Qt::Key_F) {
        return key - Qt::Key_A + 0xA;
    } else {
        return std::nullopt;
    }
}


PatternGrid::PatternGrid(PatternGridHeader &header, QWidget *parent) :
    QWidget(parent),
    mDocument(nullptr),
    mHeader(header),
    mPainter(font()),
    //mRepaintImage(true),
    mCursorRow(0),
    mCursorCol(0),
    mCursorPattern(0),
    mTrackerRow(0),
    mTrackerPattern(0),
    mFollowMode(true),
    mPatternPrev(),
    mPatternCurr(),
    mPatternNext(),
    mPatternRect(),
    mSelecting(false),
    mEditMode(false),
    mSettingShowPreviews(true),
    mVisibleRows(0)
{

    setAutoFillBackground(true);

    // first time initialization
    fontChanged();

}

int PatternGrid::row() const {
    return mCursorRow;
}

int PatternGrid::column() const {
    return mCursorCol;
}

PatternConstants::ColumnType PatternGrid::columnType() const {
    return static_cast<PatternConstants::ColumnType>(mCursorCol % TRACK_COLUMNS);
}

bool PatternGrid::isRecording() const {
    return mEditMode;
}

void PatternGrid::setRecord(bool record) {
    if (mEditMode != record) {
        mEditMode = record;
        update();
    }
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

void PatternGrid::setPreviewEnable(bool previews) {
    if (previews != mSettingShowPreviews) {
        mSettingShowPreviews = previews;
        if (mDocument) {
            setPatterns(mCursorPattern);
            if (!previews) {
                mPatternPrev.reset();
                mPatternNext.reset();
            }

            update();
        }
    }
}

void PatternGrid::setShowFlats(bool showFlats) {
    if (showFlats != mPainter.flats()) {
        mPainter.setFlats(showFlats);
        update();
    }
}

void PatternGrid::setTrackerCursor(int row, int pattern) {
    REQUIRE_DOCUMENT();

    if (mTrackerPattern != pattern) {
        mTrackerPattern = pattern;
        if (mFollowMode) {
            setCursorPattern(pattern);
            setCursorRow(row);
        }
        update();
    } else if (mTrackerRow != row) {
        mTrackerRow = row;
        if (mFollowMode) {
            setCursorRow(row);
        }
        update();
    }
}

bool PatternGrid::processKeyPress(PianoInput const& input, int const key) {
    bool validKey = false;

    switch (key) {
        case Qt::Key_Delete:
            validKey = true;
            break;
        case Qt::Key_Backspace:
            validKey = true;
            break;
        default:
        {
            auto coltype = static_cast<ColumnType>(mCursorCol % TRACK_COLUMNS);
            switch (coltype) {
                case PatternConstants::COLUMN_NOTE:
                {
                    auto note = input.keyToNote(key);
                    if (note) {
                        // TODO: preview the note
                        //mPreviewKey = key;
                        // set it in the grid
                        if (mEditMode) {
                            // TODO: set note

                        }

                        validKey = true;
                    }
                }
                    break;
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
                            // TODO edit effect type
                        }

                        validKey = true;
                    }
                }
                    break;
                case PatternConstants::COLUMN_INSTRUMENT_HIGH:
                case PatternConstants::COLUMN_INSTRUMENT_LOW:
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
                            // TODO edit instrument/effect arg
                        }

                        validKey = true;
                    }
                }
                break;
            }
        }
        break;
    }

    return validKey;
}

void PatternGrid::setDocument(ModuleDocument *doc) {
    if (mDocument) {
        mDocument->orderModel().disconnect(this);
    }


    mDocument = doc;

    if (doc) {
        auto &orderModel = doc->orderModel();
        connect(&orderModel, &OrderModel::currentTrackChanged, this, &PatternGrid::setCursorTrack);
        connect(&orderModel, &OrderModel::currentPatternChanged, this, &PatternGrid::setCursorPattern);
        connect(&orderModel, &OrderModel::patternsChanged, this, [this]() {
            setPatterns(mCursorPattern);
            update();
        });
        /*connect(&model, &SongListModel::patternSizeChanged, this,
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
            });*/

        auto &song = mDocument->mod().song();
        mPainter.setFirstHighlight(song.rowsPerBeat());
        mPainter.setSecondHighlight(song.rowsPerMeasure());
        setPatterns(orderModel.currentPattern());
        setPatternRect();
    } else {
        mPatternPrev.reset();
        mPatternCurr.reset();
        mPatternNext.reset();
    }

    update();
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
        mDocument->orderModel().selectTrack(newtrack);
    }

    mCursorCol = column;
    emit cursorColumnChanged(mCursorCol);
    update();
}

void PatternGrid::setCursorRow(int row) {
    if (mCursorRow == row) {
        return;
    }

    auto &song = mDocument->mod().song();

    if (row < 0) {
        // go to the previous pattern or wrap around to the last one
        setCursorPattern(mCursorPattern == 0 
                            ? static_cast<int>(song.order().size()) - 1 
                            : mCursorPattern - 1);
        int currCount = mPatternCurr->totalRows();
        mCursorRow = std::max(0, currCount + row);
        mPatternRect.moveTop(((mVisibleRows / 2) - mCursorRow) * mPainter.cellHeight());
    } else if (row >= static_cast<int>(mPatternCurr->totalRows())) {
        // go to the next pattern or wrap around to the first one
        int nextPattern = mCursorPattern + 1;
        setCursorPattern(nextPattern == song.order().size() ? 0 : nextPattern);
        int currCount = mPatternCurr->totalRows();
        mCursorRow = std::min(currCount - 1, row - currCount);
        mPatternRect.moveTop(((mVisibleRows / 2) - mCursorRow) * mPainter.cellHeight());
    } else {
        mPatternRect.translate(0, (mCursorRow - row) * mPainter.cellHeight());
        mCursorRow = row;
    }

    emit cursorRowChanged(mCursorRow);
    update();
}

void PatternGrid::setCursorPattern(int pattern) {
    if (mCursorPattern == pattern) {
        return;
    }

    setPatterns(pattern);

    mCursorRow = std::min(mCursorRow, static_cast<int>(mPatternCurr->totalRows()));
    mCursorPattern = pattern;

    // update selected pattern in the model
    mDocument->orderModel().selectPattern(pattern);

    update();
}

void PatternGrid::setCursorTrack(int track) {
    if (track < 0 || track > 3) {
        return;
    }
    setCursorColumn(track * TRACK_COLUMNS);
}

void PatternGrid::setFollowMode(bool follow) {
    mFollowMode = follow;
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

    // Z-order (from back to front)
    // 1. window background (drawn by qwidget)
    // 2. row background (mBackgroundPixmap)
    // 3. current row
    // 4. selection
    // 5. cursor
    // 6. pattern text (mDisplay)
    // 7. lines

    // [2] row background
    // TODO!

    // [3] current row
    mPainter.drawRowBackground(painter, mEditMode ? PatternPainter::ROW_EDIT : PatternPainter::ROW_CURRENT, centerRow);

    // [4] selection
    // TODO!

    // [5] cursor
    mPainter.drawCursor(painter, centerRow, mCursorCol);

    // [6] text
    {
        int rowYpos = 0;
        int rowsToDraw = mVisibleRows;
        int relativeRowIndex = mCursorRow - centerRow;

        if (relativeRowIndex < 0) {
            // a negative row index means we draw from the previous pattern
            if (mPatternPrev) {
                
                auto const prevRows = mPatternPrev->totalRows();
                uint16_t rowno;
                int rowIndexToPrev = relativeRowIndex + prevRows;
                if (rowIndexToPrev < 0) {
                    rowYpos += -rowIndexToPrev * rowHeight;
                    rowno = 0;
                } else {
                    rowno = (uint16_t)rowIndexToPrev;
                }
                painter.setOpacity(0.5);
                do {
                    rowYpos = mPainter.drawRow(painter, (*mPatternPrev)[rowno], rowno, rowYpos);
                    ++rowno;
                } while (rowno != prevRows);
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
        auto const currRows = mPatternCurr->totalRows();
        do {
            rowYpos = mPainter.drawRow(painter, (*mPatternCurr)[rowno], rowno, rowYpos);
            ++rowno;
            --rowsToDraw;
        } while (rowno < currRows && rowsToDraw > 0);

        if (rowsToDraw > 0 && mPatternNext) {
            // we have extra rows and a next pattern, draw it
            rowno = 0;
            auto const nextRows = mPatternNext->totalRows();
            painter.setOpacity(0.5);
            do {
                rowYpos = mPainter.drawRow(painter, (*mPatternNext)[rowno], rowno, rowYpos);
                ++rowno;
                --rowsToDraw;
            } while (rowno < nextRows && rowsToDraw > 0);
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

    REQUIRE_DOCUMENT();

    auto oldSize = evt->oldSize();
    auto newSize = evt->size();

    if (newSize.height() != oldSize.height()) {
        auto oldVisible = mVisibleRows;
        mVisibleRows = getVisibleRows();

        if (mVisibleRows != oldVisible) {
            // the number of rows visible onscreen has changed
            setPatternRect();
        }
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

        if (mPatternRect.contains(mx, my)) {
            unsigned row, column;
            mx -= mPainter.rownoWidth();
            getCursorFromMouse(mx, my, row, column);

            setCursorRow(row);
            setCursorColumn(column);
        }
        
    }
}

// ======================================================= PRIVATE METHODS ===

void PatternGrid::fontChanged() {

    mVisibleRows = getVisibleRows();
    setPatternRect();

    auto const rownoWidth = mPainter.rownoWidth();
    auto const trackWidth = mPainter.trackWidth();
    mHeader.setWidths(rownoWidth, trackWidth);
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

void PatternGrid::setPatterns(int pattern) {
    Q_ASSERT(pattern >= 0 && pattern < 256);
    auto &song = mDocument->mod().song();

    
    if (mSettingShowPreviews) {
        // get the previous pattern for preview
        if (pattern > 0) {
            mPatternPrev.emplace(song.getPattern(static_cast<uint8_t>(pattern) - 1));
        } else {
            mPatternPrev.reset();
        }
        // get the next pattern for preview
        auto nextPattern = pattern + 1;
        if (nextPattern < song.order().size()) {
            mPatternNext.emplace(song.getPattern(static_cast<uint8_t>(nextPattern)));
        } else {
            mPatternNext.reset();
        }
    }

    // update the current pattern
    mPatternCurr.emplace(song.getPattern(static_cast<uint8_t>(pattern)));
    emit patternRowsChanged(mPatternCurr->totalRows());

    
}

void PatternGrid::setPatternRect() {
    REQUIRE_DOCUMENT();

    auto const cellHeight = mPainter.cellHeight();

    mPatternRect.setX(mPainter.rownoWidth());
    mPatternRect.setY(((mVisibleRows / 2) - mCursorRow) * cellHeight);
    mPatternRect.setWidth(mPainter.trackWidth() * 4);
    mPatternRect.setHeight(cellHeight * mPatternCurr->totalRows());
}
