
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


uint32_t correctEndian(uint32_t val) {
    #ifdef __BIG_ENDIAN
    return bswap32(val);
    #else
    return val;
    #endif
}


}