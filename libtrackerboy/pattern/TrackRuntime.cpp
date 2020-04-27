
#include "trackerboy/pattern/TrackRuntime.hpp"
#include "trackerboy/note.hpp"


namespace trackerboy {


template <ChType ch>
TrackRuntime<ch>::TrackRuntime() :
    mIr(),
    mOutputMode(OutputMode::STOPPED),
    mLastInstrumentId(0),
    mLastInstrument(nullptr),
    mFreq(0)
{
}

template <ChType ch>
void TrackRuntime<ch>::reset() {
    mIr.reset();
    mOutputMode = OutputMode::STOPPED;
    mLastInstrumentId = 0;
    mLastInstrument = nullptr;
    mFreq = 0;
    // TODO: reset effect
}

template <ChType ch>
void TrackRuntime<ch>::setRow(TrackRow row, InstrumentTable &itable) {
    if (row.flags & TrackRow::COLUMN_NOTE) {
        // a note was set
        if (row.note == NOTE_CUT) {
            // cut the current note
            if (mOutputMode == OutputMode::PLAYING) {
                mOutputMode = OutputMode::STOPPED_DISABLE_MIXER;
            }
        } else {
            if (row.note <= NOTE_LAST) {
                if (mOutputMode == OutputMode::STOPPED) {
                    mOutputMode = OutputMode::PLAYING_ENABLE_MIXER;
                }
                mFreq = NOTE_FREQ_TABLE[row.note];
                mIr.reset();
            }
            // for invalid note values just no-op
        }
    }

    if (row.flags & TrackRow::COLUMN_INST) {
        // an instrument was set
        if (mLastInstrument == nullptr || mLastInstrumentId != row.instrumentId) {
            Instrument *inst = itable[row.instrumentId];
            if (inst != nullptr) {
                mLastInstrument = inst;
                mLastInstrumentId = row.instrumentId;
                mIr.setProgram(&inst->getProgram());
            }
            // if inst is null, then this is an error in the pattern data
            // the given row is attempting to use an instrument that doesn't exist
            // no-op again (the show must go on)
        }
    }

    //if (row.flags & TrackRow::COLUMN_EFFECT) {
        // an effect was set
        // TODO: effects
    //}
}

template <ChType ch>
void TrackRuntime<ch>::step(Synth &synth, WaveTable &wtable) {

    // process effect (TODO)

    // step instrument runtime
    switch (mOutputMode) {
        case OutputMode::STOPPED_DISABLE_MIXER:
            // turn off sound output for this channel
            synth.setOutputEnable(ch, Gbs::TERM_BOTH, false);
            mOutputMode = OutputMode::STOPPED;
            // fall-through
        case OutputMode::STOPPED:
            break; // do nothing
        case OutputMode::PLAYING_ENABLE_MIXER:
            // re-enable the channel output
            synth.setOutputEnable(ch, Gbs::TERM_BOTH, true);
            mOutputMode = OutputMode::PLAYING;
            // fall-through
            [[fallthrough]];
        case OutputMode::PLAYING:
            mIr.step(synth, wtable, mFreq);
            break;
    }

}

template class TrackRuntime<ChType::ch1>;
template class TrackRuntime<ChType::ch2>;
template class TrackRuntime<ChType::ch3>;
template class TrackRuntime<ChType::ch4>;



}
