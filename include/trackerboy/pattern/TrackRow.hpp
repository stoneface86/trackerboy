#pragma once

#include <cstdint>

#include "trackerboy/EffectType.hpp"


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
        COLUMN_EFFECT3 = 0x10
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
