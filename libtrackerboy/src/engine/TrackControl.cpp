
#include "trackerboy/engine/TrackControl.hpp"
#include "trackerboy/note.hpp"

#include "internal/enumutils.hpp"

#include "gbapu.hpp"


namespace trackerboy {



TrackControl::TrackControl(ChType ch, FrequencyControl &fc) :
    mOp(),
    mInstrument(),
    mFc(fc),
    mIr(),
    mDelayCounter(),
    mCutCounter(),
    mPlaying(false),
    mEnvelope(ChannelState::defaultEnvelope(ch)),
    mPanning(ChannelState::defaultPanning(ch)),
    mTimbre(ChannelState::defaultTimbre(ch))
{
}

void TrackControl::setRow(TrackRow const& row) {
    if (row.isEmpty()) {
        // empty row, do nothing
        return;
    }

    // convert the row to an operation
    // this operation gets applied in the step method after mOp.delay frames
    mOp = Operation(row);
    mDelayCounter = mOp.delay;
}

void TrackControl::step(RuntimeContext const &rc, ChannelState &state, GlobalState &global) {

    if (mDelayCounter) {
        if (*mDelayCounter == 0) {
            // apply the operation

            // global effects
            if (mOp.patternCommand != Operation::PatternCommand::none) {
                global.patternCommand = mOp.patternCommand;
                global.patternCommandParam = mOp.patternCommandParam;
            }

            if (mOp.speed) {
                global.speed = mOp.speed;
            }

            if (mOp.halt) {
                global.halt = true;
            }

            bool restartIr = false;

            if (mOp.instrument) {
                auto inst = rc.instrumentTable.getShared(*mOp.instrument);
                if (inst) {
                    mInstrument = std::move(inst);
                    restartIr = true;
                }
            }

            if (mOp.envelope) {
                state.envelope = mEnvelope = *mOp.envelope;
            }

            if (mOp.timbre) {
                state.timbre = mTimbre = *mOp.timbre;
            }

            if (mOp.panning) {
                state.panning = mPanning = *mOp.panning;
            }

            if (mOp.note) {
                restartIr = true;
                mPlaying = true;
                state.envelope = mEnvelope;
                state.timbre = mTimbre;
                state.panning = mPanning;
                mCutCounter.reset();
            }
            // explicit channel retrigger when a note/instrument is set
            state.retrigger = restartIr;

            mCutCounter = mOp.duration;

            if (restartIr && mInstrument) {
                // restart the instrument runtime
                mIr.emplace(*mInstrument);
                mFc.useInstrument(*mInstrument);
            }

            mFc.apply(mOp);

            mDelayCounter.reset();
        } else {
            --(*mDelayCounter);
        }
    }

    if (mPlaying) {

        if (mCutCounter) {
            if (*mCutCounter == 0) {
                mPlaying = false;
                mCutCounter.reset();
            } else {
                --(*mCutCounter);
            }
        }

        if (mIr) {
            mIr->step(state);
        }

        mFc.step();
        state.frequency = mFc.frequency();

    }
    
    state.playing = mPlaying;
}



// ---------------------------------------------------

ToneTrackControl::ToneTrackControl(ChType ch) :
    TrackControl(ch, mFc),
    mFc()
{
}

NoiseTrackControl::NoiseTrackControl() :
    TrackControl(ChType::ch4, mFc),
    mFc()
{
}

}
