
#include "trackerboy/note.hpp"

#include "core/clipboard/PatternClip.hpp"
#include "core/model/PatternModel.hpp"
#include "core/model/OrderModel.hpp"
#include "core/model/SongModel.hpp"

#include <QUndoCommand>
#include <QtDebug>

#include <algorithm>
#include <memory>

PatternModel::PatternModel(Module &mod, OrderModel &orderModel, SongModel &songModel, QObject *parent) :
    QObject(parent),
    mModule(mod),
    mOrderModel(orderModel),
    mCursor(),
    mCursorPattern(0),
    mRecording(false),
    mFollowing(true),
    mPlaying(false),
    mShowPreviews(true),
    mTrackerRow(0),
    mTrackerPattern(0),
    mPatternPrev(),
    mPatternCurr(mod.data().song().getPattern(0)),
    mPatternNext(),
    mHasSelection(false),
    mSelection()
{

    // TODO: Make these connections in MainWindow    
    // OrderModel must be initialized first in order for this to work!
    connect(&orderModel, &OrderModel::currentPatternChanged, this, &PatternModel::setCursorPattern);
    connect(&orderModel, &OrderModel::currentTrackChanged, this, &PatternModel::setCursorTrack);
    connect(&orderModel, &OrderModel::patternsChanged, this, [this]() {
        CursorChangeFlags flags = CursorUnchanged;
        setPatterns(mCursorPattern, flags);
        emitIfChanged(flags);
    });

    connect(&songModel, &SongModel::patternSizeChanged, this,
        [this](int rows) {
            CursorChangeFlags flags = CursorUnchanged;
            if (mCursor.row >= rows) {
                mCursor.row = rows - 1;
                flags |= CursorRowChanged;
            }
            setPatterns(mCursorPattern, flags);
            emitIfChanged(flags);
        });
}

void PatternModel::reload() {
    // patternCurr was invalidated
    mPatternCurr = mModule.data().song().getPattern(0);
    mCursorPattern = -1;
    setCursorPattern(0);
    setCursor(PatternCursor(0, 0, 0));
    deselect();
    mTrackerRow = 0;
    mTrackerPattern = 0;
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

PatternCursor PatternModel::cursor() const {
    return mCursor;
}

int PatternModel::cursorRow() const {
    return mCursor.row;
}

int PatternModel::cursorColumn() const {
    return mCursor.column;
}

int PatternModel::cursorTrack() const {
    return mCursor.track;
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

PatternSelection PatternModel::selection() const {
    return mSelection;
}

void PatternModel::setSelection(PatternCursor pos) {
    PatternAnchor anchor(pos);
    if (mHasSelection) {
        if (anchor != mSelection.end()) {
            mSelection.setEnd(anchor);
            emit selectionChanged();
        }
    } else {
        mHasSelection = true;
        // no selection, start with just the given point selected
        mSelection = PatternSelection(anchor);
        emit selectionChanged();
    }
}

void PatternModel::selectCursor() {
    setSelection(mCursor);
}

void PatternModel::selectAll() {
    // check if the entire track is selected
    auto const lastRow = (int)mPatternCurr.totalRows() - 1;
    if (mHasSelection) {
        auto iter = mSelection.iterator();
        if (
            iter.rowStart() == 0 && iter.rowEnd() == lastRow &&
            iter.trackStart() == iter.trackEnd() &&
            iter.columnStart() == 0 && iter.columnEnd() == PatternAnchor::MAX_SELECTS - 1
        ) {
            // yes, select all instead
            mHasSelection = true;
            mSelection.setStart({ 0, 0, 0 });
            mSelection.setEnd({ lastRow, PatternAnchor::MAX_SELECTS - 1, 3 });
            emit selectionChanged();
            return;
        }
    } else {
        mHasSelection = true;
    }

    // select track
    mSelection.setStart({ 0, 0, mCursor.track });
    mSelection.setEnd({ lastRow, PatternAnchor::MAX_SELECTS - 1, mCursor.track });
    emit selectionChanged();
}

void PatternModel::selectRow(int row) {
    if (row >= 0 && row < (int)mPatternCurr.totalRows()) {
        if (!mHasSelection) {
            mHasSelection = true;
            mSelection.setStart({row, 0, 0});
        }
        mSelection.setEnd({row, PatternAnchor::MAX_SELECTS - 1, PatternAnchor::MAX_TRACKS - 1});
        emit selectionChanged();
    }
}

void PatternModel::deselect() {
    if (mHasSelection) {
        mHasSelection = false;
        emit selectionChanged();
    }
}

PatternClip PatternModel::clip() {
    PatternClip clip;
    
    if (mHasSelection) {
        clip.save(mPatternCurr, mSelection);
    } else {
        clip.save(mPatternCurr, PatternSelection(mCursor));
    }

    return clip;
}

// slots -------------------------------

void PatternModel::moveCursorRow(int amount, SelectMode mode) {
    if (mPlaying && mFollowing) {
        return;
    }
    
    if (mode == SelectionModify) {
        selectCursor();
    } else if (mode == SelectionRemove) {
        deselect();
    }
    setCursorRow(mCursor.row + amount);
    if (mode == SelectionModify) {
        selectCursor();
    }
}

void PatternModel::moveCursorColumn(int amount, SelectMode mode) {
    if (mode == SelectionModify) {
        selectCursor();
    } else if (mode == SelectionRemove) {
        deselect();
    }
    setCursorColumn(mCursor.column + amount);
    if (mode == SelectionModify) {
        selectCursor();
    }
}

void PatternModel::moveCursorTrack(int amount) {
    setCursorTrack(mCursor.track + amount);
}

void PatternModel::setCursorRow(int row) {
    if (mPlaying && mFollowing) {
        return;
    }

    CursorChangeFlags flags = CursorUnchanged;
    setCursorRowImpl(row, flags);
    emitIfChanged(flags);
}
void PatternModel::setCursorRowImpl(int row, CursorChangeFlags &flags) {
    if (row == mCursor.row) {
        return;
    }

    int oldRow = mCursor.row;
    int newRow;
    if (row < 0) {
        // go to the previous pattern or wrap around to the last one
        int prevPattern;
        if (mCursorPattern == 0) {
            prevPattern = mOrderModel.rowCount() - 1;
        } else {
            prevPattern = mCursorPattern - 1;
        }
        setCursorPatternImpl(prevPattern, flags);
        newRow = std::max(0, (int)mPatternCurr.totalRows() + row);

    } else if (row >= (int)mPatternCurr.totalRows()) {
        // go to the next pattern or wrap around to the first one
        row -= mPatternCurr.totalRows();
        auto nextPattern = mCursorPattern + 1;
        if (nextPattern == mOrderModel.rowCount()) {
            nextPattern = 0;
        }
        setCursorPatternImpl(nextPattern, flags);
        newRow = std::min((int)mPatternCurr.totalRows() - 1, row);
    } else {
        newRow = row;
    }

    if (newRow != oldRow) {
        mCursor.row = newRow;
        flags |= CursorRowChanged;
    }
}

void PatternModel::setCursorColumn(int column) {
    CursorChangeFlags flags = CursorUnchanged;
    setCursorColumnImpl(column, flags);
    emitIfChanged(flags);
}

void PatternModel::setCursorColumnImpl(int column, CursorChangeFlags &flags) {
    if (column == mCursor.column) {
        return;
    }

    if (column < 0) {
        column = -column;
        auto tracks = 1 + (column / PatternCursor::MAX_COLUMNS);
        column = PatternCursor::MAX_COLUMNS - (column % PatternCursor::MAX_COLUMNS);
        setCursorTrackImpl(mCursor.track - tracks, flags);
    } else if (column >= PatternCursor::MAX_COLUMNS) {
        auto tracks = column / PatternCursor::MAX_COLUMNS;
        column = column % PatternCursor::MAX_COLUMNS;
        setCursorTrackImpl(mCursor.track + tracks, flags);
    }

    if (mCursor.column != column) {
        mCursor.column = column;
        flags |= CursorColumnChanged;
    }

}

void PatternModel::setCursorTrack(int track) {
    CursorChangeFlags flags = CursorUnchanged;
    setCursorTrackImpl(track, flags);
    emitIfChanged(flags);
}

void PatternModel::setCursorTrackImpl(int track, CursorChangeFlags &flags) {
    if (mCursor.track == track) {
        return;
    }

    if (track < 0) {
        track = PatternCursor::MAX_TRACKS - (-track % PatternCursor::MAX_TRACKS);
    } else if (track > 3) {
        track %= PatternCursor::MAX_TRACKS;
    }

    if (track != mCursor.track) {
        mCursor.track = track;
        mOrderModel.selectTrack(track);
        flags |= CursorTrackChanged;
    }
}

void PatternModel::setCursor(PatternCursor const cursor) {
    CursorChangeFlags flags = CursorUnchanged;
    if (!(mPlaying && mFollowing)) {
        setCursorRowImpl(cursor.row, flags);
    }
    setCursorColumnImpl(cursor.column, flags);
    setCursorTrackImpl(cursor.track, flags);
    emitIfChanged(flags);
}

void PatternModel::setCursorPattern(int pattern) {

    CursorChangeFlags flags = CursorUnchanged;
    setCursorPatternImpl(pattern, flags);
    emitIfChanged(flags);
}

void PatternModel::setCursorPatternImpl(int pattern, CursorChangeFlags &flags) {
    if (mCursorPattern == pattern) {
        return;
    }

    mCursorPattern = pattern;
    mOrderModel.selectPattern(pattern);
    setPatterns(pattern, flags);
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
            mOrderModel.selectPattern(pattern);
            CursorChangeFlags flags = CursorUnchanged;
            setCursorRowImpl(row, flags);
            emitIfChanged(flags);
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
            setPreviewPatterns(mOrderModel.currentPattern());
        } else {
            mPatternPrev.reset();
            mPatternNext.reset();
        }
        emit invalidated();
    }
}

void PatternModel::setPatterns(int pattern, CursorChangeFlags &flags) {
    if (mShowPreviews) {
        setPreviewPatterns(pattern);
    }
    auto &song = mModule.data().song();

    if (mShowPreviews) {
        setPreviewPatterns(pattern);
    }

    // update the current pattern
    auto oldsize = (int)mPatternCurr.totalRows();
    mPatternCurr = song.getPattern(pattern);
    auto newsize = (int)mPatternCurr.totalRows();

    if (oldsize != newsize) {
        emit patternSizeChanged(newsize);
    }

    emit invalidated();

    if (mCursor.row >= newsize) {
        mCursor.row = newsize - 1;
        flags |= CursorRowChanged;
    }
}

void PatternModel::emitIfChanged(CursorChangeFlags flags) {
    if (flags) {
        emit cursorChanged(flags);
    }
}

void PatternModel::setPreviewPatterns(int pattern) {
    auto &song = mModule.data().song();
    // get the previous pattern for preview
    if (pattern > 0) {
        mPatternPrev.emplace(song.getPattern(pattern - 1));
    } else {
        mPatternPrev.reset();
    }
    // get the next pattern for preview
    auto nextPattern = pattern + 1;
    if (nextPattern < song.order().size()) {
        mPatternNext.emplace(song.getPattern(nextPattern));
    } else {
        mPatternNext.reset();
    }
}

int PatternModel::cursorEffectNo() {
    return (mCursor.column - PatternCursor::ColumnEffect1Type) / 3;
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
            CursorChangeFlags flags = CursorUnchanged;
            setPatterns(mCursorPattern, flags);
            emitIfChanged(flags);
        } else {
            // views just need to redraw
            emit invalidated();
        }
    }

}

bool PatternModel::selectionDataIsEmpty() {
    if (mHasSelection) {
        auto iter = mSelection.iterator();

        for (auto track = iter.trackStart(); track <= iter.trackEnd(); ++track) {
            auto tmeta = iter.getTrackMeta(track);
            for (auto row = iter.rowStart(); row <= iter.rowEnd(); ++row) {
                auto const& rowdata = mPatternCurr.getTrackRow(static_cast<trackerboy::ChType>(track), (uint16_t)row);
                if (tmeta.hasColumn<PatternAnchor::SelectNote>()) {
                    if (rowdata.queryNote()) {
                        return false;
                    }
                }

                if (tmeta.hasColumn<PatternAnchor::SelectInstrument>()) {
                    if (rowdata.queryInstrument()) {
                        return false;
                    }
                }

                if (tmeta.hasColumn<PatternAnchor::SelectEffect1>()) {
                    if (rowdata.queryEffect(0)) {
                        return false;
                    }
                }

                if (tmeta.hasColumn<PatternAnchor::SelectEffect2>()) {
                    if (rowdata.queryEffect(1)) {
                        return false;
                    }
                }

                if (tmeta.hasColumn<PatternAnchor::SelectEffect3>()) {
                    if (rowdata.queryEffect(2)) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

trackerboy::TrackRow const& PatternModel::cursorTrackRow() {
    return mPatternCurr.getTrackRow(
        static_cast<trackerboy::ChType>(mCursor.track),
        (uint16_t)mCursor.row
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
        mTrack((uint8_t)(model.mCursor.track)),
        mPattern((uint8_t)model.mCursorPattern),
        mRow((uint8_t)model.mCursor.row),
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
        return mModel.mModule.data().song().getRow(
            static_cast<trackerboy::ChType>(mTrack),
            mPattern,
            (uint16_t)mRow
        );
    }

    virtual bool edit(trackerboy::TrackRow &rowdata, uint8_t data) = 0;

private:
    void setData(uint8_t data) {
        auto &rowdata = mModel.mModule.data().song().getRow(
            static_cast<trackerboy::ChType>(mTrack),
            mPattern,
            (uint16_t)mRow
        );

        bool update;
        {
            auto ctx = mModel.mModule.edit();
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


class SelectionCmd : public QUndoCommand {

protected:
    PatternModel &mModel;
    uint8_t mPattern;
    PatternClip mClip;

    explicit SelectionCmd(PatternModel &model) :
        mModel(model),
        mPattern((uint8_t)model.mCursorPattern),
        mClip()
    {
        mClip.save(model.mPatternCurr, model.mSelection);
    }

    void restore(bool update) {
        auto pattern = mModel.mModule.data().song().getPattern(mPattern);
        {
            auto ctx = mModel.mModule.edit();
            mClip.restore(pattern);
        }

        mModel.invalidate(mPattern, update);
    }

};

class DeleteSelectionCmd : public SelectionCmd {

public:

    DeleteSelectionCmd(PatternModel &model) :
        SelectionCmd(model)
    {
    }

    virtual void redo() override {
        {
            auto ctx = mModel.mModule.edit();
            // clear all set data in the selection
            auto iter = mClip.selection().iterator();
            auto pattern = mModel.mModule.data().song().getPattern(mPattern);

            for (auto track = iter.trackStart(); track <= iter.trackEnd(); ++track) {
                auto tmeta = iter.getTrackMeta(track);
                for (auto row = iter.rowStart(); row <= iter.rowEnd(); ++row) {
                    auto &rowdata = pattern.getTrackRow(static_cast<trackerboy::ChType>(track), (uint16_t)row);
                    if (tmeta.hasColumn<PatternAnchor::SelectNote>()) {
                        rowdata.note = 0;
                    }

                    if (tmeta.hasColumn<PatternAnchor::SelectInstrument>()) {
                        rowdata.instrumentId = 0;
                    }

                    if (tmeta.hasColumn<PatternAnchor::SelectEffect1>()) {
                        rowdata.effects[0] = trackerboy::NO_EFFECT;
                    }

                    if (tmeta.hasColumn<PatternAnchor::SelectEffect2>()) {
                        rowdata.effects[1] = trackerboy::NO_EFFECT;
                    }

                    if (tmeta.hasColumn<PatternAnchor::SelectEffect3>()) {
                        rowdata.effects[2] = trackerboy::NO_EFFECT;
                    }
                }
            }

        }

        mModel.invalidate(mPattern, true);
    }

    virtual void undo() override {
        restore(true);
    }


};

class PasteCmd : public QUndoCommand {

    PatternModel &mModel;
    PatternClip mSrc;
    PatternClip mPast;
    PatternCursor mPos;
    uint8_t mPattern;
    bool mMix;

public:
    PasteCmd(PatternModel &model, PatternClip const& clip, PatternCursor pos, bool mix) :
        QUndoCommand(),
        mModel(model),
        mSrc(clip),
        mPast(),
        mPos(pos),
        mPattern((uint8_t)model.mCursorPattern),
        mMix(mix)
    {
        auto region = mSrc.selection();
        region.moveTo(pos);
        region.clamp(model.mPatternCurr.size() - 1);
        mPast.save(model.mPatternCurr, region);
    }

    virtual void redo() override {
        {
            auto ctx = mModel.mModule.edit();
            auto pattern = mModel.mModule.data().song().getPattern(mPattern);
            mSrc.paste(pattern, mPos, mMix);
        }

        mModel.invalidate(mPattern, true);
    }

    virtual void undo() override {
        {
            auto ctx = mModel.mModule.edit();
            auto pattern = mModel.mModule.data().song().getPattern(mPattern);
            mPast.restore(pattern);
        }

        mModel.invalidate(mPattern, true);
    }

};


class TransposeCmd : public SelectionCmd {

    int8_t mTransposeAmount; // semitones to transpose all notes by

public:

    explicit TransposeCmd(PatternModel &model, int8_t transposeAmount) :
        SelectionCmd(model),
        mTransposeAmount(transposeAmount)
    {
    }

    virtual void redo() override {
        {
            auto ctx = mModel.mModule.edit();
            auto iter = mClip.selection().iterator();
            auto pattern = mModel.mModule.data().song().getPattern(mPattern);

            for (auto track = iter.trackStart(); track <= iter.trackEnd(); ++track) {
                auto tmeta = iter.getTrackMeta(track);
                if (!tmeta.hasColumn<PatternAnchor::SelectNote>()) {
                    continue;
                }

                for (auto row = iter.rowStart(); row <= iter.rowEnd(); ++row) {
                    auto &rowdata = pattern.getTrackRow(static_cast<trackerboy::ChType>(track), (uint16_t)row);
                    rowdata.transpose(mTransposeAmount);
                }
            }
        }

        mModel.invalidate(mPattern, false);
    }

    virtual void undo() override {
        restore(false);
    }

};

// reverse applies to a selection, but we do not inherit SelectionCmd, as its
// redo/undo actions are the same (no need to save a chunk of the selection)
class ReverseCmd : public QUndoCommand {

    PatternModel &mModel;
    PatternSelection mSelection;
    uint8_t mPattern;

public:

    explicit ReverseCmd(PatternModel &model) :
        mModel(model),
        mSelection(model.mSelection),
        mPattern((uint8_t)model.mCursorPattern)
    {
    }

    virtual void redo() override {
        reverse();
    }

    virtual void undo() override {
        // same as redo() since reversing is an involutory function
        reverse();
    }

private:

    void reverse() {
        {
            auto ctx = mModel.mModule.edit();
            auto iter = mSelection.iterator();
            auto pattern = mModel.mModule.data().song().getPattern(mPattern);

            auto midpoint = iter.rowStart() + (iter.rows() / 2);
            for (auto track = iter.trackStart(); track <= iter.trackEnd(); ++track) {
                auto tmeta = iter.getTrackMeta(track);

                int lastRow = iter.rowEnd();
                for (auto row = iter.rowStart(); row < midpoint; ++row) {
                    auto &first = pattern.getTrackRow(static_cast<trackerboy::ChType>(track), (uint16_t)row);
                    auto &last = pattern.getTrackRow(static_cast<trackerboy::ChType>(track), (uint16_t)lastRow);

                    // inefficient, but works

                    if (tmeta.hasColumn<PatternAnchor::SelectNote>()) {
                        std::swap(first.note, last.note);
                    }

                    if (tmeta.hasColumn<PatternAnchor::SelectInstrument>()) {
                        std::swap(first.instrumentId, last.instrumentId);
                    }

                    if (tmeta.hasColumn<PatternAnchor::SelectEffect1>()) {
                        std::swap(first.effects[0], last.effects[0]);
                    }

                    if (tmeta.hasColumn<PatternAnchor::SelectEffect2>()) {
                        std::swap(first.effects[1], last.effects[1]);
                    }

                    if (tmeta.hasColumn<PatternAnchor::SelectEffect3>()) {
                        std::swap(first.effects[2], last.effects[2]);
                    }

                    --lastRow;
                }
            }
        }
        mModel.invalidate(mPattern, true);
    }

};


void PatternModel::setNote(std::optional<uint8_t> note, std::optional<uint8_t> instrument) {
        
    auto &rowdata = cursorTrackRow();
    auto oldNote = rowdata.queryNote();
    auto oldInstrument = rowdata.queryInstrument();

    auto const editNote = oldNote != note;
    if (editNote && note.has_value() && *note == trackerboy::NOTE_CUT) {
        // don't set the instrument for note cuts
        instrument.reset();
    }
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
        mModule.undoStack().push(cmd);

        
    }


}

void PatternModel::setInstrument(std::optional<uint8_t> nibble) {
    auto &rowdata = cursorTrackRow();
    auto oldInstrument = rowdata.queryInstrument();
    std::optional<uint8_t> newInstrument;
    if (nibble) {
        bool const highNibble = mCursor.column == PatternCursor::ColumnInstrumentHigh;
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
        mModule.undoStack().push(cmd);
    }
}

void PatternModel::setEffectType(trackerboy::EffectType type) {
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

        auto &stack = mModule.undoStack();
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

void PatternModel::setEffectParam(uint8_t nibble) {
    auto effectNo = cursorEffectNo();
    auto &rowdata = cursorTrackRow();

    auto &oldEffect = rowdata.effects[effectNo];
    if (oldEffect.type != trackerboy::EffectType::noEffect) {
        bool isHighNibble = mCursor.column == PatternCursor::ColumnEffect1ArgHigh ||
                            mCursor.column == PatternCursor::ColumnEffect2ArgHigh ||
                            mCursor.column == PatternCursor::ColumnEffect3ArgHigh;
        auto newParam = replaceNibble(oldEffect.param, nibble, isHighNibble);
        if (newParam != oldEffect.param) {
            auto cmd = new EffectParamEditCmd(
                *this,
                (uint8_t)effectNo,
                newParam,
                oldEffect.param
            );
            cmd->setText(tr("edit effect parameter"));
            mModule.undoStack().push(cmd);
        }
    }
        
}

void PatternModel::deleteSelection() {

    if (hasSelection()) {
        // check if the selection actually has data
        if (!selectionDataIsEmpty()) {
            auto cmd = new DeleteSelectionCmd(*this);
            cmd->setText(tr("Clear selection"));
            mModule.undoStack().push(cmd);
        }
    } else {
        switch (mCursor.column) {
            case PatternCursor::ColumnNote:
                setNote({}, {});
                break;
            case PatternCursor::ColumnInstrumentHigh:
            case PatternCursor::ColumnInstrumentLow:
                setInstrument({});
                break;
            default:
                // deleting an effect parameter or type deletes the entire effect
                setEffectType(trackerboy::EffectType::noEffect);
                break;
        }
    }
    
}

void PatternModel::transpose(int amount) {

    if (amount) { // a transpose of 0 does nothing
        if (hasSelection()) {
            auto cmd = new TransposeCmd(*this, (int8_t)amount);
            cmd->setText(tr("transpose selection"));
            mModule.undoStack().push(cmd);
        } else {
            auto &rowdata = cursorTrackRow();
            auto rowcopy = rowdata;
            rowcopy.transpose(amount);
            // if the transpose resulted in a change, create and push an edit command
            if (rowcopy.note != rowdata.note) {
                auto cmd = new NoteEditCmd(*this, rowcopy.note, rowdata.note);
                cmd->setText(tr("transpose note"));
                mModule.undoStack().push(cmd);
            }

        }
    }
}

void PatternModel::reverse() {
    if (mHasSelection) {
        auto iter = mSelection.iterator();

        if (iter.rows() > 1) {
            auto cmd = new ReverseCmd(*this);
            cmd->setText("reverse");
            mModule.undoStack().push(cmd);
        }
    }
}

void PatternModel::moveSelection(PatternCursor pos) {
    if (mHasSelection) {
        auto &undoStack = mModule.undoStack();

        undoStack.beginMacro(tr("move selection"));
        auto toMove = clip();
        undoStack.push(new DeleteSelectionCmd(*this));
        undoStack.push(new PasteCmd(*this, toMove, pos, false));
        undoStack.endMacro();

        mSelection.moveTo(pos);
        mSelection.clamp((int)mPatternCurr.totalRows() - 1);
        emit selectionChanged();
    }
}

void PatternModel::paste(PatternClip const& clip, bool mix) {
    auto cmd = new PasteCmd(*this, clip, mCursor, mix);
    cmd->setText(tr("paste"));
    mModule.undoStack().push(cmd);
}
