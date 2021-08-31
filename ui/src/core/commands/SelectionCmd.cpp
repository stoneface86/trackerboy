
#include "core/commands/SelectionCmd.hpp"

#include "core/model/PatternModel.hpp"

SelectionCmd::SelectionCmd(PatternModel &model) :
    mModel(model),
    mPattern((uint8_t)model.mCursorPattern),
    mClip()
{
    mClip.save(model.mPatternCurr, model.mSelection);
}

void SelectionCmd::restore(bool update) {
    auto pattern = mModel.source()->getPattern(mPattern);
    {
        auto ctx = mModel.mModule.edit();
        mClip.restore(pattern);
    }

    mModel.invalidate(mPattern, update);
}

