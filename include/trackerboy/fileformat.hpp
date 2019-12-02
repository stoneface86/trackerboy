
#pragma once

#include <cstdint>


namespace trackerboy {

#pragma pack(push, 1)
struct TableHeader {
    uint8_t signature[4];
    uint8_t revision;
    uint8_t tableSize;
    uint8_t tableType;
    uint32_t tableOffset;
    // Table data follows

};
#pragma pack(pop)

#pragma pack(push, 1)
struct ModuleHeader {
    uint8_t signature[12];
    char version[12];
    uint8_t revision;
    char title[32];
    char artist[32];
    char copyright[32];
    uint32_t instrumentTableOffset;
    uint32_t songTableOffset;
    uint32_t waveTableOffset;
    // tables...
};
#pragma pack(pop)

extern const char *FILE_MODULE_SIGNATURE;
extern const char *FILE_TABLE_SIGNATURE;
extern const char *FILE_TERMINATOR;

static constexpr uint8_t FILE_REVISION = 0;

//
// convert native endian value to little endian
//
uint32_t toLittleEndian(uint32_t val);

//
// convert a little endian value to native endian
//
uint32_t toNativeEndian(uint32_t val);



}