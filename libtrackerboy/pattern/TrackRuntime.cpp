
#include "trackerboy/pattern/TrackRuntime.hpp"


namespace trackerboy {


TrackRuntime::TrackRuntime(ChType trackId) :
    mTrackId(trackId),
    mIr(trackId),
    mOutputMode(OutputMode::STOPPED),
    mLastInstrumentId(0),
    mLastInstrument(nullptr),
    mFreq(0)
{
}

void TrackRuntime::reset() {
    mIr.reset();
    mOutputMode = OutputMode::STOPPED;
    mLastInstrumentId = 0;
    mLastInstrument = nullptr;
    mFreq = 0;
    // TODO: reset effect
}

void TrackRuntime::setRow(TrackRow row, InstrumentTable &itable) {
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


void TrackRuntime::step(Synth &synth, WaveTable &wtable) {

    // process effect (TODO)

    // step instrument runtime
    switch (mOutputMode) {
        case OutputMode::STOPPED_DISABLE_MIXER:
            // turn off sound output for this channel
            synth.setOutputEnable(mTrackId, Gbs::TERM_BOTH, false);
            mOutputMode = OutputMode::STOPPED;
            // fall-through
        case OutputMode::STOPPED:
            break; // do nothing
        case OutputMode::PLAYING_ENABLE_MIXER:
            // re-enable the channel output
            synth.setOutputEnable(mTrackId, Gbs::TERM_BOTH, true);
            mOutputMode = OutputMode::PLAYING;
            // fall-through
        case OutputMode::PLAYING:
            mIr.step(synth, wtable, 0, mFreq);
            break;
    }

}


}