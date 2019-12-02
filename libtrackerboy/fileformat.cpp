
#include "trackerboy/fileformat.hpp"

#ifdef _MSC_VER
#include "intrin.h"
#define bswap32(val) _byteswap_ulong(val)
#elif defined(__GNUC__)
#define bswap32(val) __builtin_bswap32(val)
#else
#define bswap32(val) ((val >> 24) | ((val >> 8) & 0x0000FF00) | ((val << 8) & 0x00FF0000) | (val << 24))
#endif


namespace trackerboy {


const char *FILE_MODULE_SIGNATURE = "~TRACKERBOY~";

const char *FILE_TABLE_SIGNATURE = "TBOY";

const char *FILE_TERMINATOR = "END";



static bool isLittleEndian() {
    union {
        uint32_t i;
        char c[4];
    } endiancheck = { 0x01020304 };

    return endiancheck.c[0] == 0x04;
}


uint32_t toLittleEndian(uint32_t val) {
    if (isLittleEndian()) {
        return val;
    } else {
        return bswap32(val);
    }
}

uint32_t toNativeEndian(uint32_t val) {
    if (isLittleEndian()) {
        return val;
    } else {
        return bswap32(val);
    }
}


}