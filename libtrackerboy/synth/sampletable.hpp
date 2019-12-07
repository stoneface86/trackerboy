
#pragma once

#include <cstdint>

namespace trackerboy {

//
// sample lookup table, converts a 4-bit sample index to a 16-bit sample
// at 25% volume (so the mixer can just sum all 4 channels).
//
extern const int16_t SAMPLE_TABLE[16];


}