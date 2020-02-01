#pragma once

#include <cstdint>

#include "trackerboy/EffectType.hpp"
#include "trackerboy/note.hpp"


namespace trackerboy {

#pragma pack(push, 1)
struct TrackRow {

    enum Columns {
        COLUMN_NOTE = 0x1,
        COLUMN_INST = 0x2,
        COLUMN_VOLUME = 0x4,
        COLUMN_EFFECT1 = 0x8,
        COLUMN_EFFECT2 = 0x10,
        COLUMN_EFFECT3 = 0x20
    };

    static constexpr uint8_t EFFECT1 = 0;
    static constexpr uint8_t EFFECT2 = 1;
    static constexpr uint8_t EFFECT3 = 2;

    
    //
    // Bit 0: if set, column 1 is set (note)
    // Bit 1: if set, column 2 is set (instrument)
    // Bit 2: if set, column 3 is set (volume)
    // Bit 3: if set, column 4 is set (effect1)
    // Bit 4: if set, column 5 is set (effect2)
    // Bit 5: if set, column 6 is set (effect3)
    // Bits 6-7: Unused
    //
    uint8_t flags;

    // Column 1 - Note
    uint8_t note;

    // Column 2 - Instrument
    uint8_t instrumentId;

    // Column 3 - Volume, Column 4-6: effect 1-3

    //
    // Bits 0-3: volume setting
    // Bits 4-7: effect1 type
    //
    uint8_t effect1;

    //
    // bits 0-3: effect2 type
    // bits 4-7: effect3 type
    //
    uint8_t effect23;

    // effect parameters

    uint8_t effect1Param;

    uint8_t effect2Param;

    uint8_t effect3Param;



};
#pragma pack(pop)

}
