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

#include "trackerboy/data/Sequence.hpp"
#include "trackerboy/trackerboy.hpp"

#include <array>
#include <optional>
#include <cstddef>
#include <cstdint>

namespace trackerboy {

//
// This class handles frequency effects for a channel. 
//
class FrequencyControl {

    enum class ModType {
        none,               // no frequency modulation
        portamento,         // automatic note slide
        pitchSlide,         // frequency slides toward a target
        noteSlide,          // frequency slides toward a target note
        arpeggio            // frequency alternates between 3 notes
    };

    enum class SlideDirection {
        up,
        down
    };

public:

    class Parameters {
        friend class FrequencyControl;

    public:
        Parameters();

        void setEffect(EffectType type, uint8_t param) noexcept;

        void setNote(uint8_t note) noexcept;

        void setPitchSequence(Sequence const& seq) noexcept;

        void setArpSequence(Sequence const& seq) noexcept;

    private:

        ModType mModType;
        SlideDirection mDirection;
        uint8_t mModParam;
        std::optional<uint8_t> mVibratoParam;
        std::optional<uint8_t> mVibratoDelayParam;
        std::optional<uint8_t> mTuneParam;
        std::optional<uint8_t> mNote;

        std::optional<Sequence::Enumerator> mPitchSequence;
        std::optional<Sequence::Enumerator> mArpSequence;
    };

    uint16_t frequency() const noexcept;

    void reset() noexcept;

    void apply(Parameters const& params) noexcept;

    void step() noexcept;

protected:

    FrequencyControl(uint16_t maxFrequency, uint8_t maxNote) noexcept;

    //
    // Converts a note -> pitch
    //
    virtual uint16_t noteLookup(uint8_t note) = 0;


private:

    void finishSlide() noexcept;

    //
    // maximum pitch unit
    //
    uint16_t const mMaxFrequency;
    
    //
    // maximum note index
    //
    uint8_t const mMaxNote;

    // only 1 slide can be active note/pitch/portamento
    // arpeggio cannot be used with slides
    // (setting a slide disables arpeggio, setting arpeggio disables a slide)

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

    // instrument sequences
    int16_t mInstrumentPitch;
    Sequence::Enumerator mPitchSequence;
    Sequence::Enumerator mArpSequence;

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

//
// Frequency control for channels 1, 2 and 3. The value returned by frequency() should be
// written to the channel's frequency registers, NRx3 and NRx4.
//
class ToneFrequencyControl : public FrequencyControl {

public:

    ToneFrequencyControl() noexcept;

protected:

    uint16_t noteLookup(uint8_t note) override;

};

//
// Frequency control for CH4. The value returned by frequency() should be used to lookup the
// NR43 value via the NOTE_NOISE_TABLE, after dividing by UNITS_PER_NOTE.
//
class NoiseFrequencyControl : public FrequencyControl {

public:
    // 4 pitch units make a note
    // lowering this increases the speed of pitch slides and allows for finer tuning
    static constexpr int UNITS_PER_NOTE = 4;


    NoiseFrequencyControl() noexcept;

protected:

    uint16_t noteLookup(uint8_t note) override;

};



}
