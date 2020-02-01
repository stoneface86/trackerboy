
#pragma once

#include <cstdint>


namespace trackerboy {

enum class EffectType : uint8_t {

    // A * indicates the effect is continuous and must be turned off
    patternSetSpeed,    // set pattern speed
    patternEnd,         // switch pattern
    pitchUp,            // * pitch slide up
    pitchDown,          // * pitch slide down
    vibrato,            // * vibrato
    arp,                // * arpeggio
    off,                // turn current effect off
    setDuty,            // set channel duty (can be overriden by instrument)
    setFlag             // set a flag
    // 9/16 effects used
    
};

}
