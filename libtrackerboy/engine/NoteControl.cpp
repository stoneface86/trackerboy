
#include "trackerboy/engine/NoteControl.hpp"
#include "trackerboy/note.hpp"


namespace trackerboy {

NoteControl::NoteControl(ChType trackId) :
    mTrackId(trackId),
    mStatus(0),
    mTriggerCounter(0),
    mCutCounter(0),
    mNote(0),
    mFrequency(0)
{
}

uint16_t NoteControl::frequency() const noexcept {
    return mFrequency;
}

bool NoteControl::isPlaying() const noexcept {
    return !!(mStatus & STAT_PLAYING);
}

void NoteControl::noteTrigger(uint8_t note, uint8_t delay) {
    mNote = note;
    mStatus |= STAT_TRIGGER;
    mTriggerCounter = delay;
}

void NoteControl::noteCut(uint8_t delay) {
    mStatus |= STAT_CUT;
    mCutCounter = delay;
}

void NoteControl::reset() noexcept {
    mStatus = 0;
    mTriggerCounter = 0;
    mCutCounter = 0;
    mNote = 0;
    mFrequency = 0;
}


void NoteControl::step(Synth &synth) noexcept {

    if (!!(mStatus & STAT_TRIGGER) && mTriggerCounter-- == 0) {

        if (mNote == NOTE_CUT) {
            cut(synth);
        } else if (mNote <= NOTE_LAST) {
            synth.setOutputEnable(mTrackId, Gbs::TERM_BOTH, true);
            mStatus |= STAT_PLAYING;
            if (mTrackId != ChType::ch4) {
                mFrequency = NOTE_FREQ_TABLE[mNote];
            }
        }

        mStatus &= ~STAT_TRIGGER;
    }

    if (!!(mStatus & STAT_CUT) && mCutCounter-- == 0) {
        cut(synth);
    }

}

void NoteControl::cut(Synth &synth) noexcept {
    synth.setOutputEnable(mTrackId, Gbs::TERM_BOTH, false);
    mStatus &= ~(STAT_PLAYING | STAT_CUT);
}

}
