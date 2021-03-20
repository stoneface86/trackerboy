
#pragma once

#include <cstdint>

namespace trackerboy {


constexpr uint32_t correctEndianFast(uint32_t const val) {
    #ifdef TRACKERBOY_BIG_ENDIAN
    return ((val >> 24) | ((val >> 8) & 0x0000FF00) | ((val << 8) & 0x00FF0000) | (val << 24));
    #else
    return val;
    #endif
}

constexpr uint16_t correctEndianFast(uint16_t const val) {
    #ifdef TRACKERBOY_BIG_ENDIAN
    return ((val >> 8) | (val << 8));
    #else
    return val;
    #endif

}

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
