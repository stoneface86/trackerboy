
#include "core/commands/pattern/EraseCmd.hpp"
#include "core/model/PatternModel.hpp"

EraseCmd::EraseCmd(PatternModel &model) :
    SelectionCmd(model)
{
}

void EraseCmd::redo() {
    {
        auto ctx = mModel.mModule.edit();
        // clear all set data in the selection
        auto iter = mClip.selection().iterator();
        auto pattern = mModel.source()->getPattern(mPattern);

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

void EraseCmd::undo() {
    restore(true);
}

