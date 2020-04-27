
#pragma once

#include <cstdint>

namespace trackerboy {

//
// The speed type determines the tempo during pattern playback. Its unit is
// frames per row in Q5.3 format. Speeds with a fractional component will
// have some rows taking an extra frame.
//
using Speed = uint8_t;

// minimum possible speed, 1.0 frames per row
static constexpr Speed SPEED_MIN = 0x8;

// maximum possible speed, 31.0 frames per row
static constexpr Speed SPEED_MAX = 0xF4;


}
