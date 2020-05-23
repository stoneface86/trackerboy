
#include "trackerboy/engine/NoteControl.hpp"
#include "trackerboy/note.hpp"


namespace trackerboy {

NoteControl::NoteControl() :
    mStatus(0),
    mTriggerCounter(0),
    mCutCounter(0),
    mNote(0)
{
}

bool NoteControl::isPlaying() const noexcept {
    return !!(mStatus & STAT_PLAYING);
}

uint8_t NoteControl::note() const noexcept {
    return mNote;
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
}


void NoteControl::step() noexcept {

    if (!!(mStatus & STAT_TRIGGER) && mTriggerCounter-- == 0) {

        if (mNote == NOTE_CUT) {
            cut();
        } else if (mNote <= NOTE_LAST) {
            mStatus |= STAT_PLAYING;
        }

        mStatus &= ~STAT_TRIGGER;
    }

    if (!!(mStatus & STAT_CUT) && mCutCounter-- == 0) {
        cut();
    }

}

void NoteControl::cut() noexcept {
    mStatus &= ~(STAT_PLAYING | STAT_CUT);
}

}
