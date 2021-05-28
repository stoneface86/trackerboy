
#include "core/model/PatternModel.hpp"

#include "core/model/ModuleDocument.hpp"

#include <QUndoCommand>

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
    mPatternNext()
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

// slots -------------------------------

void PatternModel::moveCursorRow(int amount) {
    setCursorRow(mCursorRow + amount);
}

void PatternModel::moveCursorColumn(int amount) {
    setCursorColumn(mCursorColumn + amount);
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
        emit patternsChanged();
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

    emit patternsChanged();

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

// editing ====================================================================

static uint8_t replaceNibble(uint8_t value, uint8_t nibble, bool highNibble) {
    if (highNibble) {
        return (value & 0x0F) | (nibble << 4);
    } else {
        return (value & 0xF0) | (nibble);
    }
}

static constexpr bool effectTypeRequiresUpdate(trackerboy::EffectType type) {
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

//
// Command for editing a single column, ie setting a note, instrument, etc
//
class PatternEditColumnCmd : public QUndoCommand {

public:

    enum DataColumn {
        Note = offsetof(trackerboy::TrackRow, note),
        Instrument = offsetof(trackerboy::TrackRow, instrumentId),
        EffectType = offsetof(trackerboy::TrackRow, note)
    };

    PatternEditColumnCmd(
        PatternModel &model,
        uint8_t newData,
        uint8_t oldData,
        uint8_t offset,
        bool updatePatterns = false
    ) :
        mModel(model),
        mNewData(newData),
        mOldData(oldData),
        mPattern((uint8_t)model.mCursorPattern),
        mRow((uint8_t)model.mCursorRow),
        mOffset(offset),
        mUpdatePatterns(updatePatterns)
    {
    }

    virtual void redo() override {
        setData(mNewData);
    }

    virtual void undo() override {
        setData(mOldData);
    }


private:

    void setData(uint8_t data) {

        auto track = mOffset / sizeof(trackerboy::TrackRow);
        auto offsetInTrack = mOffset % sizeof(trackerboy::TrackRow);
        auto &rowdata = mModel.mDocument.mod().song().getRow(
            static_cast<trackerboy::ChType>(track),
            mPattern,
            mRow
        );

        {
            auto ctx = mModel.mDocument.beginCommandEdit();
            reinterpret_cast<uint8_t*>(&rowdata)[offsetInTrack] = data;
        }

        if (mUpdatePatterns) {
            mModel.setPatterns(mModel.mCursorPattern);
        } else {
            emit mModel.dataChanged();
        }
    }

    PatternModel &mModel;
    uint8_t const mNewData;
    uint8_t const mOldData;
    uint8_t const mPattern;
    uint8_t const mRow;
    uint8_t const mOffset;
    bool const mUpdatePatterns;

};

struct ColumnEditHelper {

    ColumnEditHelper(PatternModel &model, size_t offset) :
        track((uint8_t)model.mCursorColumn / PatternModel::COLUMNS_PER_TRACK),
        offset(track * sizeof(trackerboy::TrackRow) + offset),
        rowdata(model.mPatternCurr.getTrackRow(static_cast<trackerboy::ChType>(track), (uint16_t)model.mCursorRow))
    {
    }

    uint8_t track;
    uint8_t offset;
    trackerboy::TrackRow const& rowdata;
};

void PatternModel::setNote(std::optional<uint8_t> note) {
    if (mRecording) {
        ColumnEditHelper helper(*this, offsetof(trackerboy::TrackRow, note));
        auto oldnote = helper.rowdata.queryNote();
        if (oldnote != note) {
            auto cmd = new PatternEditColumnCmd(
                *this,
                trackerboy::TrackRow::convertColumn(note),
                trackerboy::TrackRow::convertColumn(oldnote),
                helper.offset
            );

            mDocument.undoStack().push(cmd);
        }
    }
}

void PatternModel::setInstrument(std::optional<uint8_t> nibble) {
    if (mRecording) {
        ColumnEditHelper helper(*this, offsetof(trackerboy::TrackRow, instrumentId));

        auto oldInstrument = helper.rowdata.queryInstrument();
        std::optional<uint8_t> newInstrument;
        if (nibble) {
            bool const highNibble = columnType() == COLUMN_INSTRUMENT_HIGH;
            newInstrument = replaceNibble(oldInstrument.value_or((uint8_t)0), *nibble, highNibble);
        }

        if (newInstrument != oldInstrument) {
            auto cmd = new PatternEditColumnCmd(
                *this,
                trackerboy::TrackRow::convertColumn(newInstrument),
                trackerboy::TrackRow::convertColumn(oldInstrument),
                helper.offset
            );

            mDocument.undoStack().push(cmd);
        }

    }
}

void PatternModel::setEffectType(trackerboy::EffectType type) {
    if (mRecording) {
        auto effectNo = cursorEffectNo();
        ColumnEditHelper helper(*this, offsetof(trackerboy::TrackRow, effects) + (effectNo * sizeof(trackerboy::Effect)));

        auto& oldEffect = helper.rowdata.effects[effectNo];
        if (oldEffect.type != type) {
            mDocument.undoStack().push(new PatternEditColumnCmd(
                *this,
                (uint8_t)type,
                (uint8_t)oldEffect.type,
                helper.offset,
                effectTypeRequiresUpdate(type) || effectTypeRequiresUpdate(oldEffect.type)
            ));
        }
    }
}

void PatternModel::setEffectParam(uint8_t nibble) {
    if (mRecording) {
        auto effectNo = cursorEffectNo();
        ColumnEditHelper helper(*this, offsetof(trackerboy::TrackRow, effects) + 1 + (effectNo * sizeof(trackerboy::Effect)));

        auto &oldEffect = helper.rowdata.effects[effectNo];
        if (oldEffect.type != trackerboy::EffectType::noEffect) {
            auto coltype = columnType();
            bool isHighNibble = coltype == COLUMN_EFFECT1_ARG_HIGH ||
                                coltype == COLUMN_EFFECT2_ARG_HIGH ||
                                coltype == COLUMN_EFFECT3_ARG_HIGH;
            auto newParam = replaceNibble(oldEffect.param, nibble, isHighNibble);
            if (newParam != oldEffect.param) {
                mDocument.undoStack().push(new PatternEditColumnCmd(
                    *this,
                    newParam,
                    oldEffect.param,
                    helper.offset
                ));
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
                setNote({});
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