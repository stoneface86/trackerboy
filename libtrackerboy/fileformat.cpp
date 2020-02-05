
#include "trackerboy/fileformat.hpp"

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


const char *FILE_SIGNATURE = "~TRACKERBOY~";


uint32_t correctEndian(uint32_t val) {
    #ifdef TRACKERBOY_BIG_ENDIAN
    return bswap32(val);
    #else
    return val;
    #endif
}

uint16_t correctEndian(uint16_t val) {
    #ifdef TRACKERBOY_BIG_ENDIAN
    return bswap16(val);
    #else
    return val;
    #endif
}

float correctEndian(float val) {
    static_assert(sizeof(float) == sizeof(uint32_t), "sizeof(float) != 4");

    #ifdef TRACKERBOY_BIG_ENDIAN
    union {
        float f;
        uint32_t i;
    } temp;

    temp.f = val;
    temp.i = bswap32(temp.i);
    return temp.f;
    #else
    return val;
    #endif
}


}
