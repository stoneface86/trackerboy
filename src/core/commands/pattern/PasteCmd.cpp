
#include "core/commands/pattern/PasteCmd.hpp"
#include "core/model/PatternModel.hpp"

PasteCmd::PasteCmd(
    PatternModel &model,
    PatternClip const& clip,
    PatternCursor pos,
    bool mix
) :
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

void PasteCmd::redo() {
    {
        auto ctx = mModel.mModule.edit();
        auto pattern = mModel.source()->getPattern(mPattern);
        mSrc.paste(pattern, mPos, mMix);
    }

    mModel.invalidate(mPattern, true);
}

void PasteCmd::undo() {
    {
        auto ctx = mModel.mModule.edit();
        auto pattern = mModel.source()->getPattern(mPattern);
        mPast.restore(pattern);
    }

    mModel.invalidate(mPattern, true);
}
