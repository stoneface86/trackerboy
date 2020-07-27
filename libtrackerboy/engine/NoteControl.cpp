
#include "trackerboy/engine/NoteControl.hpp"
#include "trackerboy/note.hpp"


namespace trackerboy {

NoteControl::NoteControl() :
    mPlaying(false),
    mTriggerCounter(0),
    mCutCounter(0),
    mNote(0)
{
}

bool NoteControl::isPlaying() const noexcept {
    return mPlaying;
}

void NoteControl::noteTrigger(uint8_t note, uint8_t delay) noexcept {
    if (note == NOTE_CUT) {
        mCutCounter = delay + 1;
    } else {
        mNote = note;
        mTriggerCounter = delay + 1;
    }
}

void NoteControl::noteCut(uint8_t delay) noexcept {
    mCutCounter = delay + 1;
}

void NoteControl::reset() noexcept {
    mPlaying = false;
    mTriggerCounter = 0;
    mCutCounter = 0;
    mNote = 0;
}


std::optional<uint8_t> NoteControl::step() noexcept {

    bool trigger = false;

    if (mTriggerCounter && --mTriggerCounter == 0) {
        mPlaying = true;
        trigger = true;
    }

    if (mCutCounter && --mCutCounter == 0) {
        mPlaying = false;
        trigger = false;
    }

    return (trigger) ? std::optional<uint8_t>{mNote} : std::nullopt;

}


}
