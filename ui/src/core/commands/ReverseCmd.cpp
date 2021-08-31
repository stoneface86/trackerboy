
#include "core/commands/ReverseCmd.hpp"
#include "core/model/PatternModel.hpp"


ReverseCmd::ReverseCmd(PatternModel &model) :
    mModel(model),
    mSelection(model.mSelection),
    mPattern((uint8_t)model.mCursorPattern)
{
}

void ReverseCmd::redo() {
    reverse();
}

void ReverseCmd::undo() {
    // same as redo() since reversing is an involutory function
    reverse();
}

void ReverseCmd::reverse() {
    {
        auto ctx = mModel.mModule.edit();
        auto iter = mSelection.iterator();
        auto pattern = mModel.source()->getPattern(mPattern);

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

