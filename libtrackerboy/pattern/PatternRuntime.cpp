
#include "trackerboy/Table.hpp"
#include "trackerboy/pattern/PatternRuntime.hpp"


namespace trackerboy {


PatternRuntime::PatternRuntime(ChType trackId) :
    mPattern(nullptr),
    mIr(trackId),
    mTrackId(trackId),
    mIsNewRow(false),
    mIsPlaying(false),
    mLastInstrumentId(0),
    mLastInstrument(nullptr),
    mFreq(0)
{
}

bool PatternRuntime::nextRow() {
    if (mPattern != nullptr) {
        if (++mTrack == mTrackEnd) {
            return true;
        }

        mCurrentRow = *mTrack;
        mIsNewRow = true;
    }
    return false;
}

void PatternRuntime::reset() {
    if (mPattern != nullptr) {
        mTrack = mPattern->trackBegin(mTrackId);
        mTrackEnd = mPattern->trackEnd(mTrackId);
        if (mTrack != mTrackEnd) {
            mCurrentRow = *mTrack;
            mIsNewRow = true;
        }
    }

    mIsPlaying = false;
    mLastInstrument = nullptr;
    mLastInstrumentId = 0;
    mFreq = 0;
    // TODO: reset effects
}

void PatternRuntime::setPattern(Pattern *pattern) {
    mPattern = pattern;
    reset();
}

bool PatternRuntime::step(Synth &synth, InstrumentTable &itable, WaveTable &wtable) {

    if (mTrack == mTrackEnd) {
        return true;
    } else {

        if (mIsNewRow) {
            if (mCurrentRow.flags & TrackRow::COLUMN_NOTE) {
                // a note was set
                if (mCurrentRow.note == NOTE_CUT) {
                    // cut the current note
                    if (mIsPlaying) {
                        synth.getMixer().setEnable(mTrackId, Gbs::TERM_BOTH, false);
                        mIsPlaying = false;
                    }
                } else {
                    if (mCurrentRow.note <= NOTE_LAST) {
                        if (!mIsPlaying) {
                            // re-enable the channel output
                            synth.getMixer().setEnable(mTrackId, Gbs::TERM_BOTH, true);
                            mIsPlaying = true;
                        }
                        mFreq = NOTE_FREQ_TABLE[mCurrentRow.note];
                    }
                    // for invalid note values just no-op
                    // TODO: throw exception? or just warn?
                }
            }

            if (mCurrentRow.flags & TrackRow::COLUMN_INST) {
                // an instrument was set
                if (mLastInstrument == nullptr || mLastInstrumentId != mCurrentRow.instrumentId) {
                    Instrument *inst = itable[mCurrentRow.instrumentId];
                    if (inst != nullptr) {
                        mLastInstrument = inst;
                        mLastInstrumentId = mCurrentRow.instrumentId;
                        mIr.setProgram(&inst->getProgram());
                    }
                    // if mLastInstrument is null, then this is an error in the pattern data
                    // the given row is attempting to use an instrument that doesn't exist
                    // TODO: throw/warn?
                }
            }

            if (mCurrentRow.flags & TrackRow::COLUMN_EFFECT) {
                // an effect was set
                // TODO: effects
            }

            mIsNewRow = false;
        }

        // process effect (TODO)

        // step instrument runtime
        if (mIsPlaying) {
            mIr.step(synth, wtable, 0, mFreq);
        }
    }

    // keep going
    return false;

}

}