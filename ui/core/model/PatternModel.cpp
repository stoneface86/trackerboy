
#include "core/model/PatternModel.hpp"

#include "core/model/ModuleDocument.hpp"

#include <QUndoCommand>
#include <QtDebug>

#include <algorithm>
#include <memory>


static constexpr size_t columnToOffset(int column) {
    switch (column) {
        case PatternModel::SelectNote:
            return offsetof(trackerboy::TrackRow, note);
        case PatternModel::SelectInstrument:
            return offsetof(trackerboy::TrackRow, instrumentId);
        case PatternModel::SelectEffect1:
            return offsetof(trackerboy::TrackRow, effects) + sizeof(trackerboy::Effect) * 0;
        case PatternModel::SelectEffect2:
            return offsetof(trackerboy::TrackRow, effects) + sizeof(trackerboy::Effect) * 1;
        default:
            return offsetof(trackerboy::TrackRow, effects) + sizeof(trackerboy::Effect) * 2;
    }
}

class SelectedTrackMeta {

    int mColumnStart;
    int mColumnEnd;

public:

    constexpr SelectedTrackMeta(int start, int end) :
        mColumnStart(start),
        mColumnEnd(end)
    {
    }

    constexpr int columnStart() const {
        return mColumnStart;
    }

    constexpr int columnEnd() const {
        return mColumnEnd;
    }

    size_t length() const {
        size_t offset = columnToOffset(mColumnStart);
        if (mColumnEnd % PatternModel::SELECTS_PER_TRACK == 0) {
            return sizeof(trackerboy::TrackRow) - offset;
        } else {
            return columnToOffset(mColumnEnd) - offset;
        }
    }

    template <PatternModel::SelectType column>
    bool hasColumn() {
        return column >= mColumnStart && column <= mColumnEnd;
    }

};

class SelectionMeta {

    int mRowStart;
    int mRowEnd;
    int mColumnStart;
    int mColumnEnd;
    int mTrackStart;
    int mTrackEnd;


public:
    

    explicit SelectionMeta(QRect rect) :
        mRowStart(rect.top()),
        mRowEnd(rect.bottom()),
        mColumnStart(rect.left()),
        mColumnEnd(rect.right()),
        mTrackStart(rect.left() / PatternModel::SELECTS_PER_TRACK),
        mTrackEnd((rect.right() - 1) / PatternModel::SELECTS_PER_TRACK + 1)
    {
    }

    size_t patternRowOffset() const {
        return (sizeof(trackerboy::TrackRow) * mTrackStart) + columnToOffset(mColumnStart % PatternModel::SELECTS_PER_TRACK);
    }

    size_t patternRowLength() const {
        size_t length = sizeof(trackerboy::TrackRow) * (mTrackEnd - mTrackStart - 1);
        if (mColumnEnd % PatternModel::SELECTS_PER_TRACK == 0) {
            length += sizeof(trackerboy::TrackRow);
        } else {
            length += columnToOffset(mColumnEnd % PatternModel::SELECTS_PER_TRACK);
        }
        return length - columnToOffset(mColumnStart % PatternModel::SELECTS_PER_TRACK);
    }

    constexpr int rowStart() const {
        return mRowStart;
    }

    constexpr int rowEnd() const {
        return mRowEnd;
    }

    constexpr int rows() const {
        return mRowEnd - mRowStart;
    }

    constexpr int trackStart() const {
        return mTrackStart;
    }

    constexpr int trackEnd() const {
        return mTrackEnd;
    }

    SelectedTrackMeta getTrackMeta(int track) {
        int start;
        int end;

        if (track == mTrackStart) {
            start = mColumnStart % PatternModel::SELECTS_PER_TRACK;
        } else {
            start = 0;
        }

        if (track == mTrackEnd - 1) {
            end = (mColumnEnd - 1) % PatternModel::SELECTS_PER_TRACK;
            end++;
        } else {
            end = PatternModel::SELECTS_PER_TRACK - 1;
        }

        return { start, end };

    }


};

//
// Container class for a chunk of pattern data that can be saved/restored from
// a given selection.
//
class PatternChunk {

    std::unique_ptr<char[]> mData;

    //
    // Moves data from chunk to pattern (saving = false) or pattern to chunk (saving = true)
    //
    void move(trackerboy::Pattern &pattern, QRect selection, bool saving) {
        SelectionMeta smeta(selection);
        auto rowLength = smeta.patternRowLength();

        char *dest, *src;

        if (saving) {
            mData.reset(new char[rowLength * smeta.rows()]);
        }

        // for saving, the destination is the chunk, and the source is the pattern
        // for restoring, the destination is the pattern and the source is the chunk
        char* &patternPtr = (saving) ? src : dest;
        char* &chunkPtr = (saving) ? dest : src;
        
        char *buf = mData.get();
        for (int track = smeta.trackStart(); track < smeta.trackEnd(); ++track) {

            auto tmeta = smeta.getTrackMeta(track);
            auto offset = columnToOffset(tmeta.columnStart());
            auto length = tmeta.length();
            // with this assertion passing, we will never read or write past
            // the bounds of a TrackRow
            Q_ASSERT(offset + length <= sizeof(trackerboy::TrackRow));

            chunkPtr = buf;
            for (int row = smeta.rowStart(); row < smeta.rowEnd(); ++row) {
                auto &rowdata = pattern.getTrackRow(static_cast<trackerboy::ChType>(track), (uint16_t)row);
                patternPtr = reinterpret_cast<char*>(&rowdata) + offset;
                std::copy_n(src, length, dest);
                
                chunkPtr += rowLength;
            }

            // advance to next track
            buf += length;

        }
    }

public:
    explicit PatternChunk() :
        mData()
    {

    }

    //
    // Restores a previously saved chunk to the given pattern. 
    //
    void restore(trackerboy::Pattern &dest, QRect selection) {
        if (!mData) {
            return;
        }
        move(dest, selection, false);
    }

    //
    // Saves the selected data from the given pattern
    //
    void save(trackerboy::Pattern const& src, QRect selection) {
        // remove const for src, but we can gurantee that it will not be
        // modified since we are saving
        move(const_cast<trackerboy::Pattern&>(src), selection, true);
    }

};


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
            return SelectNote;
        case COLUMN_INSTRUMENT_HIGH:
        case COLUMN_INSTRUMENT_LOW:
            return SelectInstrument;
        case COLUMN_EFFECT1_TYPE:
        case COLUMN_EFFECT1_ARG_HIGH:
        case COLUMN_EFFECT1_ARG_LOW:
            return SelectEffect1;
        case COLUMN_EFFECT2_TYPE:
        case COLUMN_EFFECT2_ARG_HIGH:
        case COLUMN_EFFECT2_ARG_LOW:
            return SelectEffect2;
        default:
            return SelectEffect3;
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

void PatternModel::selectRow(int row) {
    if (row >= 0 && row < (int)mPatternCurr.totalRows()) {
        if (!mHasSelection) {
            setSelection({0, row});
        }
        setSelection({ SELECTS - 1, row});
    }
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

bool PatternModel::selectionDataIsEmpty() {
    if (mHasSelection) {
        SelectionMeta smeta(selection());

        for (auto track = smeta.trackStart(); track < smeta.trackEnd(); ++track) {
            auto tmeta = smeta.getTrackMeta(track);
            for (auto row = smeta.rowStart(); row < smeta.rowEnd(); ++row) {
                auto const& rowdata = mPatternCurr.getTrackRow(static_cast<trackerboy::ChType>(track), (uint16_t)row);
                if (tmeta.hasColumn<PatternModel::SelectNote>()) {
                    if (rowdata.queryNote()) {
                        return false;
                    }
                }

                if (tmeta.hasColumn<PatternModel::SelectInstrument>()) {
                    if (rowdata.queryInstrument()) {
                        return false;
                    }
                }

                if (tmeta.hasColumn<PatternModel::SelectEffect1>()) {
                    if (rowdata.queryEffect(0)) {
                        return false;
                    }
                }

                if (tmeta.hasColumn<PatternModel::SelectEffect2>()) {
                    if (rowdata.queryEffect(1)) {
                        return false;
                    }
                }

                if (tmeta.hasColumn<PatternModel::SelectEffect3>()) {
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









class DeleteSelectionCmd : public QUndoCommand {

    PatternModel &mModel;
    uint8_t mPattern;
    QRect mSelection;
    PatternChunk mChunk;

public:

    DeleteSelectionCmd(PatternModel &model) :
        mModel(model),
        mPattern((uint8_t)model.mCursorPattern),
        mSelection(model.selection()),
        mChunk()
    {
        mChunk.save(model.mPatternCurr, mSelection);
    }

    virtual void redo() override {
        {
            auto ctx = mModel.mDocument.beginCommandEdit();
            // clear all set data in the selection
            SelectionMeta smeta(mSelection);
            qDebug() << smeta.patternRowLength();
            auto pattern = mModel.mDocument.mod().song().getPattern(mPattern);

            for (auto track = smeta.trackStart(); track < smeta.trackEnd(); ++track) {
                auto tmeta = smeta.getTrackMeta(track);
                for (auto row = smeta.rowStart(); row < smeta.rowEnd(); ++row) {
                    auto &rowdata = pattern.getTrackRow(static_cast<trackerboy::ChType>(track), (uint16_t)row);
                    if (tmeta.hasColumn<PatternModel::SelectNote>()) {
                        rowdata.note = 0;
                    }

                    if (tmeta.hasColumn<PatternModel::SelectInstrument>()) {
                        rowdata.instrumentId = 0;
                    }

                    if (tmeta.hasColumn<PatternModel::SelectEffect1>()) {
                        rowdata.effects[0] = trackerboy::NO_EFFECT;
                    }

                    if (tmeta.hasColumn<PatternModel::SelectEffect2>()) {
                        rowdata.effects[1] = trackerboy::NO_EFFECT;
                    }

                    if (tmeta.hasColumn<PatternModel::SelectEffect3>()) {
                        rowdata.effects[2] = trackerboy::NO_EFFECT;
                    }
                }
            }

        }

        mModel.invalidate(mPattern, true);
    }

    virtual void undo() override {
        auto pattern = mModel.mDocument.mod().song().getPattern(mPattern);
        {
            auto ctx = mModel.mDocument.beginCommandEdit();
            mChunk.restore(pattern, mSelection);
        }

        mModel.invalidate(mPattern, true);
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

    if (hasSelection()) {
        // check if the selection actually has data
        if (!selectionDataIsEmpty()) {
            auto cmd = new DeleteSelectionCmd(*this);
            cmd->setText(tr("Clear selection"));
            mDocument.undoStack().push(cmd);
        }
    } else {
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
    }
    
}