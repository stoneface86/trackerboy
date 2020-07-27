
#pragma once

#include <cstdint>
#include <optional>

#include "trackerboy/synth/Synth.hpp"


namespace trackerboy {


class NoteControl {

public:

    NoteControl();

    //
    // Returns true if a note is currently playing.
    //
    bool isPlaying() const noexcept;

    //
    // The note from last trigger. Should only be accessed when step() returned true.
    //
    //uint8_t note() const noexcept;

    //
    // Trigger the given note in the given number of frames.
    //
    void noteTrigger(uint8_t note, uint8_t delay = 0) noexcept;

    //
    // Cut the note in the given number of frames.
    //
    void noteCut(uint8_t delay = 0) noexcept;

    //
    // Reset the control to its initial state.
    //
    void reset() noexcept;

    //
    // Update note and cut counters. If a note is to be triggered on this step,
    // it is returned.
    //
    std::optional<uint8_t> step() noexcept;

private:

    bool mPlaying;

    // a counter value of 0 means there is no note or cut to be triggered
    uint8_t mTriggerCounter;    // frames until the note gets triggered
    uint8_t mCutCounter;        // frames until the note gets cut

    uint8_t mNote;

};


}
