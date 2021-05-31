
#include "core/model/PatternModel.hpp"

#include "core/model/ModuleDocument.hpp"

#include <QUndoCommand>
#include <QtDebug>

PatternModel::PatternModel(ModuleDocument &doc, QObject *parent) :
    QObject(parent),
    mDocument(doc),
    mCursorRow(0),
    mCursorColumn(0),
    mCursorPattern(0),
    mRecording(false),
    mFollowing(true),
    mPlaying(false),
    mShowPreviews(true),
    mTrackerRow(0),
    mTrackerPattern(0),
    mPatternPrev(),
    mPatternCurr(doc.mod().song().getPattern(0)),
    mPatternNext(),
    mHasSelection(false),
    mSelection()
{
    // OrderModel must be initialized first in order for this to work!
    auto &orderModel = doc.orderModel();
    connect(&orderModel, &OrderModel::currentPatternChanged, this, &PatternModel::setCursorPattern);
    connect(&orderModel, &OrderModel::currentTrackChanged, this, &PatternModel::setCursorTrack);
    connect(&orderModel, &OrderModel::patternsChanged, this, [this]() {
        setPatterns(mCursorPattern);
    });

    auto &songModel = doc.songModel();
    connect(&songModel, &SongModel::patternSizeChanged, this,
        [this](int rows) {
            if (mCursorRow >= rows) {
                mCursorRow = rows - 1;
                emit cursorRowChanged(mCursorRow);
            }
            setPatterns(mCursorPattern);
        });
}

void PatternModel::reload() {
    // patternCurr was invalidated
    mPatternCurr = mDocument.mod().song().getPattern(0);
    setCursorPattern(0);
}

trackerboy::Pattern* PatternModel::previousPattern() {
    return mPatternPrev ? &*mPatternPrev : nullptr;
}

trackerboy::Pattern& PatternModel::currentPattern() {
    return mPatternCurr;
}

trackerboy::Pattern* PatternModel::nextPattern() {
    return mPatternNext ? &*mPatternNext : nullptr;
}

int PatternModel::cursorRow() const {
    return mCursorRow;
}

int PatternModel::cursorColumn() const {
    return mCursorColumn;
}

PatternModel::ColumnType PatternModel::columnType() const {
    return static_cast<ColumnType>(mCursorColumn % COLUMNS_PER_TRACK);
}

PatternModel::SelectType PatternModel::selectType() const {
    switch (columnType()) {
        case COLUMN_NOTE:
            return SELECT_NOTE;
        case COLUMN_INSTRUMENT_HIGH:
        case COLUMN_INSTRUMENT_LOW:
            return SELECT_INSTRUMENT;
        case COLUMN_EFFECT1_TYPE:
        case COLUMN_EFFECT1_ARG_HIGH:
        case COLUMN_EFFECT1_ARG_LOW:
            return SELECT_EFFECT1;
        case COLUMN_EFFECT2_TYPE:
        case COLUMN_EFFECT2_ARG_HIGH:
        case COLUMN_EFFECT2_ARG_LOW:
            return SELECT_EFFECT2;
        default:
            return SELECT_EFFECT3;
    }
}

int PatternModel::trackerCursorRow() const {
    return mTrackerRow;
}

int PatternModel::trackerCursorPattern() const {
    return mTrackerPattern;
}

bool PatternModel::isRecording() const {
    return mRecording;
}

bool PatternModel::isFollowing() const {
    return mFollowing;
}

bool PatternModel::isPlaying() const {
    return mPlaying;
}

bool PatternModel::hasSelection() const {
    return mHasSelection;
}

QRect PatternModel::selection() const {
    if (mHasSelection) {
        // selection start is the topLeft, end is bottomRight
        // alter the rectangle such that the topLeft is < bottomRight in both dimensions
        auto x1 = mSelection.left();
        auto x2 = mSelection.right();
        auto y1 = mSelection.top();
        auto y2 = mSelection.bottom();
        if (x1 > x2) {
            std::swap(x1, x2);
        }
        if (y1 > y2) {
            std::swap(y1, y2);
        }
        return {
            QPoint(x1, y1),
            QPoint(x2 + 1, y2 + 1)
        };
    } else {
        return {};
    }
}

// there is some weirdness with QRect but keep this in mind:
// mSelection keeps the starting coordinate in QRect::topLeft()
// and keeps the ending coordinate in QRect::bottomRight()

void PatternModel::setSelection(QPoint const point) {
    if (mHasSelection) {
        if (point != mSelection.bottomRight()) {
            mSelection.setBottomRight(point);
            emit selectionChanged();
        }
    } else {
        mHasSelection = true;
        // no selection, start with just the given point selected
        mSelection = QRect(point, point);
        emit selectionChanged();
    }
}

void PatternModel::selectCursor() {
    auto track = mCursorColumn / COLUMNS_PER_TRACK;
    auto select = selectType();
    setSelection({track * SELECTS_PER_TRACK + select, mCursorRow});
}

void PatternModel::selectAll() {
    // check if the entire track is selected
    auto const lastRow = (int)mPatternCurr.totalRows() - 1;
    if (mHasSelection) {
        auto normalized = selection();
        // is an entire track and only an entire track selected?
        if (normalized.top() == 0 && (normalized.left() % SELECTS_PER_TRACK) == 0 &&
            normalized.bottom() == lastRow + 1 &&
            (normalized.right() % SELECTS_PER_TRACK) == 0 &&
            normalized.width() == SELECTS_PER_TRACK + 1) {
                // yes, select all instead
                mHasSelection = true;
                mSelection.setTopLeft({0, 0});
                mSelection.setBottomRight({SELECTS - 1, lastRow});
                emit selectionChanged();
                return;
            }
    }

    // select track
    auto selectStart = mCursorColumn / COLUMNS_PER_TRACK * SELECTS_PER_TRACK;
    mSelection.setTopLeft({selectStart, 0});
    mSelection.setBottomRight({selectStart + SELECTS_PER_TRACK - 1, lastRow});
    mHasSelection = true;
    emit selectionChanged();
}

void PatternModel::deselect() {
    if (mHasSelection) {
        mHasSelection = false;
        mSelection = QRect();
        emit selectionChanged();
    }
}

// slots -------------------------------

void PatternModel::moveCursorRow(int amount, bool select) {
    if (select) {
        selectCursor();
    } else {
        deselect();
    }
    setCursorRow(mCursorRow + amount);
    if (select) {
        selectCursor();
    }
}

void PatternModel::moveCursorColumn(int amount, bool select) {
    if (select) {
        selectCursor();
    } else {
        deselect();
    }
    setCursorColumn(mCursorColumn + amount);
    if (select) {
        selectCursor();
    }
}

void PatternModel::setCursorRow(int row) {
    if (mCursorRow == row) {
        return;
    }

    auto &orderModel = mDocument.orderModel();
    auto const pattern = orderModel.currentPattern();

    if (row < 0) {
        // go to the previous pattern or wrap around to the last one
        orderModel.selectPattern(pattern == 0 ? orderModel.rowCount() - 1 : pattern - 1);
        mCursorRow = std::max(0, (int)mPatternCurr.totalRows() + row);

    } else if (row >= (int)mPatternCurr.totalRows()) {
        // go to the next pattern or wrap around to the first one
        row -= mPatternCurr.totalRows();
        auto nextPattern = pattern + 1;
        if (nextPattern == orderModel.rowCount()) {
            nextPattern = 0;
        }
        orderModel.selectPattern(nextPattern);
        mCursorRow = std::min((int)mPatternCurr.totalRows() - 1, row);
    } else {
        mCursorRow = row;
    }

    emit cursorRowChanged(mCursorRow);
}

void PatternModel::setCursorColumn(int column) {
    if (mCursorColumn == column) {
        return;
    }

    if (column < 0) {
        column = COLUMNS - (-column % COLUMNS);
    } else if (column >= COLUMNS) {
        column = column % COLUMNS;
    }

    int track = mCursorColumn / COLUMNS_PER_TRACK;
    int newtrack = column / COLUMNS_PER_TRACK;
    if (track != newtrack) {
        mDocument.orderModel().selectTrack(newtrack);
    }

    mCursorColumn = column;
    emit cursorColumnChanged(mCursorColumn);
}

void PatternModel::setCursorTrack(int track) {
    Q_ASSERT(track >= 0 && track < 4);

    auto col = track * COLUMNS_PER_TRACK;
    if (mCursorColumn != col) {
        mCursorColumn = col;
        emit cursorColumnChanged(col);
    }
}

void PatternModel::setCursorPattern(int pattern) {

    if (mCursorPattern == pattern) {
        return;
    }

    setPatterns(pattern);
    mCursorPattern = pattern;
    deselect();

}

void PatternModel::setTrackerCursor(int row, int pattern) {
    bool changed = false;
    if (mTrackerPattern != pattern) {
        mTrackerPattern = pattern;
        changed = true;
    }
    if (mTrackerRow != row) {
        mTrackerRow = row;
        changed = true;
    }

    if (changed) {
        if (mFollowing) {
            mDocument.orderModel().selectPattern(pattern);
            setCursorRow(row);
        }
        emit trackerCursorChanged(row, pattern);
    }
}

void PatternModel::setFollowing(bool following) {
    mFollowing = following;
}

void PatternModel::setPlaying(bool playing) {
    if (mPlaying != playing) {
        mPlaying = playing;
        emit playingChanged(playing);
    }
}

void PatternModel::setRecord(bool record) {
    if (mRecording != record) {
        mRecording = record;
        emit recordingChanged(record);
    }
}

void PatternModel::setPreviewEnable(bool enable) {
    if (mShowPreviews != enable) {
        mShowPreviews = enable;
        if (enable) {
            setPreviewPatterns(mDocument.orderModel().currentPattern());
        } else {
            mPatternPrev.reset();
            mPatternNext.reset();
        }
        emit invalidated();
    }
}

void PatternModel::setPatterns(int pattern) {
    if (mShowPreviews) {
        setPreviewPatterns(pattern);
    }
    auto &song = mDocument.mod().song();

    if (mShowPreviews) {
        setPreviewPatterns(pattern);
    }

    // update the current pattern
    auto oldsize = (int)mPatternCurr.totalRows();
    mPatternCurr = song.getPattern((uint8_t)pattern);
    auto newsize = (int)mPatternCurr.totalRows();

    if (oldsize != newsize) {
        emit patternSizeChanged(newsize);
    }

    emit invalidated();

    if (mCursorRow >= newsize) {
        mCursorRow = newsize - 1;
        emit cursorRowChanged(mCursorRow);
    }
}

void PatternModel::setPreviewPatterns(int pattern) {
    auto &song = mDocument.mod().song();
    // get the previous pattern for preview
    if (pattern > 0) {
        mPatternPrev.emplace(song.getPattern((uint8_t)pattern - 1));
    } else {
        mPatternPrev.reset();
    }
    // get the next pattern for preview
    auto nextPattern = pattern + 1;
    if (nextPattern < song.order().size()) {
        mPatternNext.emplace(song.getPattern((uint8_t)nextPattern));
    } else {
        mPatternNext.reset();
    }
}

int PatternModel::cursorEffectNo() {
    return (columnType() - COLUMN_EFFECT1_TYPE) / 3;
}

void PatternModel::invalidate(int pattern, bool updatePatterns) {

    // check if the pattern being invalidated is accessible
    bool isInvalid = (mCursorPattern == pattern) ||
                     (mPatternPrev && pattern == mCursorPattern - 1) ||
                     (mPatternNext && pattern == mCursorPattern + 1);

    if (isInvalid) {
        // pattern is now invalid, either reset the pattern accessors
        // or send the invalidated signal
        if (updatePatterns) {
            // reset pattern accessors and invalidate
            setPatterns(mCursorPattern);
        } else {
            // views just need to redraw
            emit invalidated();
        }
    }

}

trackerboy::TrackRow const& PatternModel::cursorTrackRow() {
    return mPatternCurr.getTrackRow(
        static_cast<trackerboy::ChType>(mCursorColumn / COLUMNS_PER_TRACK),
        (uint16_t)mCursorRow
    );
}

// editing ====================================================================

static uint8_t replaceNibble(uint8_t value, uint8_t nibble, bool highNibble) {
    if (highNibble) {
        return (value & 0x0F) | (nibble << 4);
    } else {
        return (value & 0xF0) | (nibble);
    }
}

static constexpr bool effectTypeRequiresUpdate(trackerboy::EffectType type) {
    // these effects shorten the length of a pattern which when set/removed
    // will require a recount
    return type == trackerboy::EffectType::patternHalt ||
           type == trackerboy::EffectType::patternSkip ||
           type == trackerboy::EffectType::patternGoto;
}


// static void copyFromPattern(trackerboy::Pattern const& src, uint16_t rowno, int start, int count, char *dest) {
//     int channel = start / sizeof(trackerboy::TrackRow);
//     int offset = start % sizeof(trackerboy::TrackRow);
//     while (count) {
//         auto const& rowdata = src.getTrackRow(static_cast<trackerboy::ChType>(channel), rowno);
//         auto toCopy = std::min(count, (int)sizeof(trackerboy::TrackRow) - offset);
//         std::copy_n(reinterpret_cast<const char*>(&rowdata) + offset, toCopy, dest);
//         dest += toCopy;
//         offset = 0;
//         ++channel;
//         count -= toCopy;
//     }
// }

// static void copyToPattern(const char* src, uint16_t rowno, int colFrom, int count, trackerboy::Pattern &dest) {
//     int channel = colFrom / sizeof(trackerboy::TrackRow);
//     int offset = colFrom % sizeof(trackerboy::TrackRow);
//     while (count) {
//         auto &rowdata = dest.getTrackRow(static_cast<trackerboy::ChType>(channel), rowno);
//         auto toCopy = std::min(count, (int)sizeof(trackerboy::TrackRow) - offset);
//         std::copy_n(src, toCopy, reinterpret_cast<char*>(&rowdata) + offset);
//         src += toCopy;
//         offset = 0;
//         ++channel;
//         count -= toCopy;
//     }
// }

class TrackEditCmd : public QUndoCommand {

protected:
    PatternModel &mModel;
    uint8_t const mTrack;
    uint8_t const mPattern;
    uint8_t const mRow;
    uint8_t const mNewData;
    uint8_t const mOldData;

public:
    TrackEditCmd(PatternModel &model, uint8_t dataNew, uint8_t dataOld, QUndoCommand *parent = nullptr) :
        QUndoCommand(parent),
        mModel(model),
        mTrack((uint8_t)(model.mCursorColumn / PatternModel::COLUMNS_PER_TRACK)),
        mPattern((uint8_t)model.mCursorPattern),
        mRow((uint8_t)model.mCursorRow),
        mNewData(dataNew),
        mOldData(dataOld)
    {
    }

    virtual void redo() override {
        setData(mNewData);
    }

    virtual void undo() override {
        setData(mOldData);
    }

protected:
    trackerboy::TrackRow& getRow() {
        return mModel.mDocument.mod().song().getRow(
            static_cast<trackerboy::ChType>(mTrack),
            mPattern,
            (uint16_t)mRow
        );
    }

    virtual bool edit(trackerboy::TrackRow &rowdata, uint8_t data) = 0;

private:
    void setData(uint8_t data) {
        auto &rowdata = mModel.mDocument.mod().song().getRow(
            static_cast<trackerboy::ChType>(mTrack),
            mPattern,
            (uint16_t)mRow
        );

        bool update;
        {
            auto ctx = mModel.mDocument.beginCommandEdit();
            update = edit(rowdata, data);
        }

        mModel.invalidate(mPattern, update);

    }

};

class NoteEditCmd : public TrackEditCmd {

    using TrackEditCmd::TrackEditCmd;

protected:
    virtual bool edit(trackerboy::TrackRow &rowdata, uint8_t data) override {
        rowdata.note = data;
        return false;
    }
};

class InstrumentEditCmd : public TrackEditCmd {

    using TrackEditCmd::TrackEditCmd;

protected:
    virtual bool edit(trackerboy::TrackRow &rowdata, uint8_t data) override {
        rowdata.instrumentId = data;
        return false;
    }

};

class EffectEditCmd : public TrackEditCmd {

public:
    EffectEditCmd(PatternModel &model, uint8_t effectNo, uint8_t newData, uint8_t oldData, QUndoCommand *parent = nullptr) :
        TrackEditCmd(model, newData, oldData, parent),
        mEffectNo(effectNo)
    {
    }

protected:
    uint8_t const mEffectNo;
};

class EffectTypeEditCmd : public EffectEditCmd {

    using EffectEditCmd::EffectEditCmd;

protected:
    virtual bool edit(trackerboy::TrackRow &rowdata, uint8_t data) override {
        auto &effect = rowdata.effects[mEffectNo];
        auto oldtype = effect.type;
        auto type = static_cast<trackerboy::EffectType>(data);
        effect.type = type;
        return effectTypeRequiresUpdate(type) || effectTypeRequiresUpdate(oldtype);
    }

};

class EffectParamEditCmd : public EffectEditCmd {

    using EffectEditCmd::EffectEditCmd;

protected:
    virtual bool edit(trackerboy::TrackRow &rowdata, uint8_t data) override {
        rowdata.effects[mEffectNo].param = data;
        return false;
    }

};


void PatternModel::setNote(std::optional<uint8_t> note, std::optional<uint8_t> instrument) {
    if (mRecording) {
        
        auto &rowdata = cursorTrackRow();
        auto oldNote = rowdata.queryNote();
        auto oldInstrument = rowdata.queryInstrument();

        auto const editNote = oldNote != note;
        // edit the instrument if the instrument has a value and the it does not equal the current instrument
        auto const editInstrument = instrument && oldInstrument != instrument;
        int editCount = editNote + editInstrument;
        if (editCount) {
            QUndoCommand *parent = nullptr;
            QUndoCommand *cmd = nullptr;

            if (editCount == 2) {
                parent = new QUndoCommand;
            }

            if (editNote) {
                cmd = new NoteEditCmd(
                    *this,
                    trackerboy::TrackRow::convertColumn(note),
                    trackerboy::TrackRow::convertColumn(oldNote),
                    parent
                );
            }

            if (editInstrument) {
                cmd = new InstrumentEditCmd(
                    *this,
                    trackerboy::TrackRow::convertColumn(instrument),
                    trackerboy::TrackRow::convertColumn(oldInstrument),
                    parent
                );
            }

            if (parent) {
                cmd = parent;
            }

            if (note) {
                cmd->setText(tr("Note entry")); // todo: put the pattern, row, and track in this text
            } else {
                cmd->setText(tr("Clear note"));
            }
            mDocument.undoStack().push(cmd);

            
        }

    }
}

void PatternModel::setInstrument(std::optional<uint8_t> nibble) {
    if (mRecording) {
        auto &rowdata = cursorTrackRow();
        auto oldInstrument = rowdata.queryInstrument();
        std::optional<uint8_t> newInstrument;
        if (nibble) {
            bool const highNibble = columnType() == COLUMN_INSTRUMENT_HIGH;
            newInstrument = replaceNibble(oldInstrument.value_or((uint8_t)0), *nibble, highNibble);
            if (*newInstrument >= trackerboy::MAX_INSTRUMENTS) {
                return;
            }
        }

        if (newInstrument != oldInstrument) {
            auto cmd = new InstrumentEditCmd(
                *this,
                trackerboy::TrackRow::convertColumn(newInstrument),
                trackerboy::TrackRow::convertColumn(oldInstrument)
            );
            if (newInstrument) {
                cmd->setText(tr("set instrument"));
            } else {
                cmd->setText(tr("clear instrument"));
            }
            mDocument.undoStack().push(cmd);
        }

    }
}

void PatternModel::setEffectType(trackerboy::EffectType type) {
    if (mRecording) {
        auto effectNo = cursorEffectNo();
        auto &rowdata = cursorTrackRow();
        auto &effect = rowdata.effects[effectNo];
        if (effect.type != type) {
            auto cmd = new EffectTypeEditCmd(
                *this,
                (uint8_t)effectNo,
                static_cast<uint8_t>(type),
                static_cast<uint8_t>(effect.type)
            );

            auto &stack = mDocument.undoStack();
            if (type == trackerboy::EffectType::noEffect) {

                static auto CLEAR_EFFECT_STR = QT_TR_NOOP("clear effect");

                // we also need to clear the parameter
                if (effect.param != 0) {
                    stack.beginMacro(tr(CLEAR_EFFECT_STR));
                    stack.push(cmd);
                    stack.push(new EffectParamEditCmd(
                        *this, (uint8_t)effectNo, 0, effect.param
                    ));
                    stack.endMacro();
                } else {
                    cmd->setText(tr(CLEAR_EFFECT_STR));
                    stack.push(cmd);
                }
                
            } else {
                cmd->setText(tr("set effect type"));
                stack.push(cmd);
            }
        }
    }
}

void PatternModel::setEffectParam(uint8_t nibble) {
    if (mRecording) {
        auto effectNo = cursorEffectNo();
        auto &rowdata = cursorTrackRow();

        auto &oldEffect = rowdata.effects[effectNo];
        if (oldEffect.type != trackerboy::EffectType::noEffect) {
            auto coltype = columnType();
            bool isHighNibble = coltype == COLUMN_EFFECT1_ARG_HIGH ||
                                coltype == COLUMN_EFFECT2_ARG_HIGH ||
                                coltype == COLUMN_EFFECT3_ARG_HIGH;
            auto newParam = replaceNibble(oldEffect.param, nibble, isHighNibble);
            if (newParam != oldEffect.param) {
                auto cmd = new EffectParamEditCmd(
                    *this,
                    (uint8_t)effectNo,
                    newParam,
                    oldEffect.param
                );
                cmd->setText(tr("edit effect parameter"));
                mDocument.undoStack().push(cmd);
            }
        }
        
    }
}

void PatternModel::deleteSelection() {

    // TODO: check for selection and delete it
    // if (has selection) {

    //} else {
        switch (columnType()) {
            case COLUMN_NOTE:
                setNote({}, {});
                break;
            case COLUMN_INSTRUMENT_HIGH:
            case COLUMN_INSTRUMENT_LOW:
                setInstrument({});
                break;
            default:
                // deleting an effect parameter or type deletes the entire effect
                setEffectType(trackerboy::EffectType::noEffect);
                break;
        }
    //}
    
}