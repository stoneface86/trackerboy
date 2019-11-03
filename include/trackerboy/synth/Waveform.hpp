
#pragma once

#include <cstdint>

#include "trackerboy/gbs.hpp"


namespace trackerboy {

struct Waveform {
    uint8_t data[Gbs::WAVE_RAMSIZE];
};

// note: a struct is being used instead of typedef'ing an array
//       https://stackoverflow.com/a/4523537

}