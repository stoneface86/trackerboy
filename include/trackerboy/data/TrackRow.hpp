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

#include "trackerboy/EffectType.hpp"

#include <cstdint>

namespace trackerboy {

#pragma pack(push, 1)

struct Effect {
    EffectType type;
    uint8_t param;
};


struct TrackRow {

    enum Columns {
        COLUMN_NOTE = 0x1,
        COLUMN_INST = 0x2,
        COLUMN_EFFECT1 = 0x4,
        COLUMN_EFFECT2 = 0x8,
        COLUMN_EFFECT3 = 0x10,
        COLUMN_EFFECTS = COLUMN_EFFECT1 | COLUMN_EFFECT2 | COLUMN_EFFECT3
    };

    static constexpr uint8_t EFFECT1 = 0;
    static constexpr uint8_t EFFECT2 = 1;
    static constexpr uint8_t EFFECT3 = 2;

    static constexpr uint8_t MAX_EFFECTS = 3;

    //
    // Bit 0: if set, column 1 is set (note)
    // Bit 1: if set, column 2 is set (instrument)
    // Bit 2: if set, column 3 is set (effect1)
    // Bit 3: if set, column 4 is set (effect2)
    // Bit 4: if set, column 5 is set (effect3)
    // Bits 5-7: Unused
    //
    uint8_t flags;

    // Column 1 - Note
    uint8_t note;

    // Column 2 - Instrument
    uint8_t instrumentId;

    // Column 3 - Effect 1
    Effect effects[MAX_EFFECTS];

};

#pragma pack(pop)


}
