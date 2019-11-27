
#pragma once

#include <cstdint>

namespace trackerboy {

using Q53 = uint8_t;

//
// creates a literal uint8_t in Q53 format
// q53(1,0) ==> 00001.000 (1.0)
// q53(3,6) ==> 00011.110 (3.75)
//
constexpr Q53 Q53_make(uint8_t integral, uint8_t fraction) {
    return ((integral << 3) | (fraction & 0x7));
}

}
