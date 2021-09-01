
#include "core/commands/pattern/TrackEditCmd.hpp"

#include "core/model/PatternModel.hpp"

#define TU TrackEditCmdTU
namespace TU {

static constexpr bool effectTypeRequiresUpdate(trackerboy::EffectType type) {
    // these effects shorten the length of a pattern which when set/removed
    // will require a recount
    return type == trackerboy::EffectType::patternHalt ||
           type == trackerboy::EffectType::patternSkip ||
           type == trackerboy::EffectType::patternGoto;
}

}


TrackEditCmd::TrackEditCmd(
    PatternModel &model,
    uint8_t dataNew,
    uint8_t dataOld,
    QUndoCommand *parent
) :
    QUndoCommand(parent),
    mModel(model),
    mTrack((uint8_t)(model.mCursor.track)),
    mPattern((uint8_t)model.mCursorPattern),
    mRow((uint8_t)model.mCursor.row),
    mNewData(dataNew),
    mOldData(dataOld)
{
}

void TrackEditCmd::redo() {
    setData(mNewData);
}

void TrackEditCmd::undo() {
    setData(mOldData);
}

trackerboy::TrackRow& TrackEditCmd::getRow() {
    return mModel.source()->getRow(
        static_cast<trackerboy::ChType>(mTrack),
        mPattern,
        (uint16_t)mRow
    );
}

void TrackEditCmd::setData(uint8_t data) {
    auto &rowdata = mModel.source()->getRow(
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

// ===

bool NoteEditCmd::edit(trackerboy::TrackRow &rowdata, uint8_t data) {
    rowdata.note = data;
    return false;
}

// ===

bool InstrumentEditCmd::edit(trackerboy::TrackRow &rowdata, uint8_t data) {
    rowdata.instrumentId = data;
    return false;
}

// ===

EffectEditCmd::EffectEditCmd(
    PatternModel &model,
    uint8_t effectNo,
    uint8_t newData,
    uint8_t oldData,
    QUndoCommand *parent
) :
    TrackEditCmd(model, newData, oldData, parent),
    mEffectNo(effectNo)
{
}

bool EffectTypeEditCmd::edit(trackerboy::TrackRow &rowdata, uint8_t data)  {
    auto &effect = rowdata.effects[mEffectNo];
    auto oldtype = effect.type;
    auto type = static_cast<trackerboy::EffectType>(data);
    effect.type = type;
    return TU::effectTypeRequiresUpdate(type) || TU::effectTypeRequiresUpdate(oldtype);
}

// ===

bool EffectParamEditCmd::edit(trackerboy::TrackRow &rowdata, uint8_t data) {
    rowdata.effects[mEffectNo].param = data;
    return false;
}


#undef TU
