///
/// tbb.h - TrackerBoy Backend
///
/// This header provides public API functions of the TrackerBoy Backend.
///
/// Naming conventions
/// * types - pascal case prefixed with a 'B', ie, `BMyType`
/// * functions - camel case prefixed with a 'b', ie, `bMyFunction`
///
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// no specific calling convention, use compiler default
#define PUB extern

/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ CORE

typedef int8_t    BI8;
typedef uint8_t   BU8;
typedef int16_t   BI16;
typedef uint16_t  BU16;
typedef int32_t   BI32;
typedef uint32_t  BU32;
typedef int64_t   BI64;
typedef uint64_t  BU64;
typedef float     BF32;
typedef double    BF64;

//
// Generic slice of data
//
typedef struct BSlice BSlice;
struct BSlice {
    size_t len;
    const char *data;
};

///
/// Nim string
///
typedef struct BNimStr BNimStr;

///
/// String type whose data may reside in a Nim string.
///
typedef struct BStr BStr;
struct BStr {
    BNimStr *owner;
    size_t len;
    char *data;
};


///
/// Function pointer type for handling a panic, or an unhandled Nim exception
///
typedef void (*BPanicCallback)(BSlice msg);

PUB void bInit(void);

PUB void bUninit(void);

PUB void bSetPanicCallback(BPanicCallback callback);

PUB const char* bVersion(void);

PUB BStr bStrNew(void);

PUB void bStrSet(BStr *str, const char *data);

PUB void bStrDestroy(BStr *str);

#ifdef __cplusplus
}
#endif
#undef PUB
