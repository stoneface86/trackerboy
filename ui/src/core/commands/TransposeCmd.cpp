
#include "core/commands/TransposeCmd.hpp"
#include "core/model/PatternModel.hpp"


TransposeCmd::TransposeCmd(PatternModel &model, int8_t transposeAmount) :
    SelectionCmd(model),
    mTransposeAmount(transposeAmount)
{
}

void TransposeCmd::redo()  {
    {
        auto ctx = mModel.mModule.edit();
        auto iter = mClip.selection().iterator();
        auto pattern = mModel.source()->getPattern(mPattern);

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

void TransposeCmd::undo() {
    restore(false);
}
