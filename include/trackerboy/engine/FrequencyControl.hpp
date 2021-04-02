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

#include <array>
#include <optional>
#include <cstddef>
#include <cstdint>

namespace trackerboy {

//
// This class handles frequency effects for a channel. 
//
class FrequencyControl {

public:

    enum class SlideDirection {
        up,
        down
    };

    FrequencyControl() noexcept;

    uint16_t frequency() const noexcept;

    void reset() noexcept;

    // Effects 1xx and 2xx
    void setPitchSlide(SlideDirection dir, uint8_t param) noexcept;

    // Effects Qxy and Rxy
    void setNoteSlide(SlideDirection dir, uint8_t parameter) noexcept;

    // Effect 4xy
    void setVibrato(uint8_t vibrato) noexcept;

    // Effect 5xx
    void setVibratoDelay(uint8_t delay) noexcept;

    // Effect 0xy
    void setArpeggio(uint8_t param) noexcept;

    // Effect 3xx
    void setPortamento(uint8_t param) noexcept;

    void setNote(uint8_t note) noexcept;

    // Effect Pxx
    void setTune(uint8_t param) noexcept;

    void apply() noexcept;

    void step() noexcept;

private:

    enum class ModType {
        none,               // no frequency modulation
        portamento,
        pitchSlide,         // frequency slides toward a target
        noteSlide,
        arpeggio            // frequency alternates between 3 notes
    };

    struct ModEffect {

        ModType type;
        SlideDirection direction;
        uint8_t parameter;

        constexpr ModEffect(ModType type, uint8_t parameter, SlideDirection dir = SlideDirection::down) :
            type(type),
            parameter(parameter),
            direction(dir)
        {
        }

    };

    void finishSlide() noexcept;
    void setChord() noexcept;

    // only 1 slide can be active note/pitch/portamento
    // arpeggio cannot be used with slides
    // (setting a slide disables arpeggio, setting arpeggio disables a slide)

    bool mNewNote;

    std::optional<ModEffect> mEffectToApply;

    // the current modulation effect
    ModType mMod;

    // the current note
    uint8_t mNote;

    // pitch offset
    int8_t mTune;
    // the current frequency
    int16_t mFrequency;

    // pitch slide

    uint8_t mSlideAmount;
    uint16_t mSlideTarget;


    // arpeggio

    uint8_t mChordOffset1;
    uint8_t mChordOffset2;

    // index in the chord array
    unsigned mChordIndex;
    // note frequencies for the arpeggio "chord"
    std::array<uint16_t, 3> mChord;

    // vibrato
    bool mVibratoEnabled;
    uint8_t mVibratoDelayCounter;
    uint8_t mVibratoCounter;

    int8_t mVibratoValue;   // current offset value of the vibrato (+/- vibrato depth)
    uint8_t mVibratoDelay;  // vibrato delay setting
    uint8_t mVibratoParam;  // vibrato effect parameter (upper nibble is speed, lower is depth)
};



}
