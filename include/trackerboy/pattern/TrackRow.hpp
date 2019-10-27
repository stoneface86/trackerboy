#pragma once

#include <cstdint>

#include "trackerboy/EffectType.hpp"
#include "trackerboy/note.hpp"


namespace trackerboy {

struct TrackRow {

    enum Columns {
        COLUMN_NOTE = 0x1,
        COLUMN_INST = 0x2,
        COLUMN_EFFECT = 0x4
    };

    //
    // Bit 0: if set, column 1 is set (note)
    // Bit 1: if set, column 2 is set (instrument)
    // Bit 2: if set, column 3 is set (effect)
    // Bit 3: unused
    // Bits 4-7: Effect type
    //
    uint8_t flags;

    // Column 1 - Note
    uint8_t note;

    // Column 2 - Instrument
    uint8_t instrumentId;

    // Column 3 - Effect setting
    uint8_t effectParameter;


};

}