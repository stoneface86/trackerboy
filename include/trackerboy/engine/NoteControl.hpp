/*
** Trackerboy - Gameboy / Gameboy Color music tracker
** Copyright (C) 2019-2020 stoneface86
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
*/

#pragma once

#include <cstdint>
#include <optional>

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
