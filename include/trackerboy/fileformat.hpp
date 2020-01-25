
#pragma once

#include <cstdint>
#include <fstream>
#include <string>


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


// new header format
//
//     +0     +1     +2     +3
// 0   +---------------------------+
//     |                           |
//     | signature (~TRACKERBOY~)  |
//     |                           |
// 12  +---------------------------+
//     | version major             |
// 16  +---------------------------+
//     | version minor             |
// 20  +---------------------------+
//     | version patch             |
// 24  +------+------+-------------+
//     | rev  | type |             |
//     +------+------+             |
//     |                           |
//     |                           |
//     |                           |
//     | title                     |
//     |                           |
//     |                           |
//     |                           |
// 56  +---------------------------+
//     |                           |
//     |                           |
//     |                           |
//     | artist                    |
//     |                           |
//     |                           |
//     |            +--------------|
//     |            |              |
// 96  +------------+              |
//     |                           |
//     |                           |
//     |                           |
//     | copyright                 |
//     |                           |
//     |                           |
//     |                           |
// 128 +---------------------------+
//     |                           |
//     |                           |
//     |                           |
//     | reserved                  |
//     |                           |
//     |                           |
//     |                           |
// 156 +---------------------------+
//     | chunk size                |
//     +---------------------------+
//
// the chunk then follows (contents depend on type)

#pragma pack(push, 1)
struct Header {

    static constexpr size_t SIGNATURE_SIZE = 12;
    static constexpr size_t ARTIST_LENGTH = 32;
    static constexpr size_t TITLE_LENGTH = 32;
    static constexpr size_t COPYRIGHT_LENGTH = 32;

    uint8_t signature[SIGNATURE_SIZE];
    uint32_t versionMajor;
    uint32_t versionMinor;
    uint32_t versionPatch;
    uint8_t revision;
    uint8_t type;
    char title[TITLE_LENGTH];
    char artist[ARTIST_LENGTH];
    char copyright[COPYRIGHT_LENGTH];
    // 28 bytes (7 words) are reserved for future revisions of the format
    uint32_t reserved[7];
    // chunkSize starts at offset 156
    uint32_t chunkSize;
};
#pragma pack(pop)

enum class ChunkType : uint8_t {

    // chunk for a module file, contains chunk data for instrument, song
    // and wave tables.
    mod = 0,

    // instrument table chunk, item data for an instrument table
    instrument = 1,

    // song table chunk, item data for a song table
    song = 2,

    // waveform table chunk, item data for a waveform table
    wave = 3,

    last = wave

};



enum class FormatError {
    none,
    invalidSignature,
    invalidRevision,
    invalidTableCode,
    badOffset,
    invalidTerminator,
    readError,
    writeError
};

extern const char *FILE_MODULE_SIGNATURE;
extern const char *FILE_TABLE_SIGNATURE;
extern const char *FILE_TERMINATOR;

//
// Integer representing the current revision of the trackerboy file format.
// Starts at 0 (initial version) and is incremented on every change to the
// format.
//
static constexpr uint8_t FILE_REVISION = 0;


//
// Change the endian of the given value if needed. On big endian
// systems, the value is byte swapped. On little endian systems
// the same value is returned.
//
uint32_t correctEndian(uint32_t val);

float correctEndian(float val);


}