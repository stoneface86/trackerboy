
#include "internal/endian.hpp"

#ifdef _MSC_VER
#include "intrin.h"
#define bswap32(val) _byteswap_ulong(val)
#define bswap16(val) _byteswap_ushort(val)
#elif defined(__GNUC__)
#define bswap32(val) __builtin_bswap32(val)
#define bswap16(val) __builtin_bswap16(val)
#else
#define bswap32(val) ((val >> 24) | ((val >> 8) & 0x0000FF00) | ((val << 8) & 0x00FF0000) | (val << 24))
#define bswap16(val) ((val >> 8) | (val << 8))
#endif

namespace trackerboy {

#ifdef TRACKERBOY_BIG_ENDIAN

uint32_t correctEndian(uint32_t val) {
    return bswap32(val);
}

uint16_t correctEndian(uint16_t val) {
    return bswap16(val);

}

float correctEndian(float const val) {
    static_assert(sizeof(float) == sizeof(uint32_t), "sizeof(float) != 4");

    union {
        float f;
        uint32_t i;
    } temp;

    temp.f = val;
    temp.i = bswap32(temp.i);
    return temp.f;

}

#endif


}
