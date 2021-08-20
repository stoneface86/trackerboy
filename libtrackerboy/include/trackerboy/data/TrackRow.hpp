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

#include "trackerboy/trackerboy.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace trackerboy {

#pragma pack(push, 1)

struct Effect {
    EffectType type;
    uint8_t param;
};


struct TrackRow {

    static constexpr uint8_t MAX_EFFECTS = 3;

    // Column 1 - Note
    // nonzero value is the note index plus 1
    // a zero indicates the column has no note set
    uint8_t note;

    // Column 2 - Instrument
    // same as notes, nonzero values is instrument id plus 1
    // zero has no instrument set
    uint8_t instrumentId;

    // Column 3 - Effect 1
    std::array<Effect, MAX_EFFECTS> effects;

    bool isEmpty() const noexcept;

    std::optional<uint8_t> queryNote() const noexcept;

    std::optional<uint8_t> queryInstrument() const noexcept;
    
    std::optional<Effect> queryEffect(size_t effectNo) const noexcept;

    void setNote(std::optional<uint8_t> note);

    void setInstrument(std::optional<uint8_t> instrument);

    void transpose(int semitones);

    // converts a note or instrument optional to its value stored in this struct
    static constexpr uint8_t convertColumn(std::optional<uint8_t> value) {
        return value.value_or((uint8_t)-1) + 1;
    }

};

#pragma pack(pop)

constexpr Effect NO_EFFECT = { EffectType::noEffect, 0 };


}
