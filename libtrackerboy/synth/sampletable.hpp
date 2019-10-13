
#pragma once

#include <cstdint>

namespace trackerboy {

//
// sample lookup table, converts a 4-bit sample index to a 16-bit sample
// at 25% volume (so the mixer can just sum all 4 channels). Every 16
// samples in this table are associated with an envelope volume, starting
// at 0xF (max) to 0x0 (min).
//
// example: to get the sample for 0xF (-1) at envelope volume 0xD
// int16_t sample = SAMPLE_TABLE[(0xF - 0xD) * 16 + 0xF]
//
extern const int16_t SAMPLE_TABLE[16 * 16];


}