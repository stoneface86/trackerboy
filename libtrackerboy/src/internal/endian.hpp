
#pragma once

#include <cstdint>

namespace trackerboy {

#ifdef TRACKERBOY_BIG_ENDIAN

// big endian system, these functions will "correct" endianness by byte swapping

uint32_t correctEndian(uint32_t const val);

uint16_t correctEndian(uint16_t const val);

float correctEndian(float const val);

#else

// little-endian already so we don't need to byte swap
// convert all correctEndian function calls to do nothing
#define correctEndian(val) (val)

#endif

}
