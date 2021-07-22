/*
** Trackerboy - Gameboy / Gameboy Color music tracker
** Copyright (C) 2019-2020 stoneface86
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
*/

#pragma once

#include <cstdint>
#include <cstddef>

namespace trackerboy {

// new header format
//
//     +0         +1         +2        +3
// 0   +-------------------------------------------+
//     |                                           |
//     | signature ( TRACKERBOY )                  |
//     |                                           |
// 12  +-------------------------------------------+
//     | version major                             |
// 16  +-------------------------------------------+
//     | version minor                             |
// 20  +-------------------------------------------+
//     | version patch                             |
// 24  +----------+----------+---------------------+
//     | rev      |
//     +----------+
//
//     ======= FORMAT BELOW DEPENDS ON REV! ========
//
//                +----------+---------------------+
//                | system   | customFramerate     |
// 28  +----------+----------+---------------------+
//     |                                           |
//     |                                           |
//     |                                           |
//     | title                                     |
//     |                                           |
//     |                                           |
//     |                                           |
//     |                                           |
// 60  +-------------------------------------------+
//     |                                           |
//     |                                           |
//     |                                           |
//     | artist                                    |
//     |                                           |
//     |                                           |
//     |                                           |
//     |                                           |
// 92  +-------------------------------------------|
//     |                                           |
//     |                                           |
//     |                                           |
//     | copyright                                 |
//     |                                           |
//     |                                           |
//     |                                           |
//     |                                           |
// 124 +----------+----------+----------+----------+
//     | icount   | scount   | wcount   | reserved |
// 128 +---------------------+----------+----------+
//     |                                           |
//     |                                           |
//     |                                           |
//     |                                           |
//     | reserved                                  |
//     |                                           |
//     |                                           |
//     |                                           |
//     |                                           |
// 160 +-------------------------------------------+
//

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

    // ========================================================================

    uint8_t system; // DMG, SGB, custom
    uint16_t customFramerate; // framerate if system == System::custom
    char title[TITLE_LENGTH];
    char artist[ARTIST_LENGTH];
    char copyright[COPYRIGHT_LENGTH];
    uint8_t icount;
    uint8_t scount;
    uint8_t wcount;
    // remainder is reserved for future revisions of the format
    uint32_t reserved[8];

    
};
#pragma pack(pop)



enum class FormatError {
    none,                   // no error
    invalidSignature,       // signature does not match
    invalidRevision,        // unsupported file revision
    cannotUpgrade,          // module from previous revision is not upgradable
    duplicateId,            // two instruments/waveforms with the same id
    invalid,                // data format is invalid
    unknownChannel,         // unknown channel id for track data
    readError,              // read error occurred
    writeError              // write error occurred
};

// All data past the header is contained within "blocks"
// A block starts with a 4-byte identifier (BlockId) and
// a 4-byte length (BlockSize). The identifier defines the
// type of data.

using BlockId = uint32_t;
using BlockSize = uint32_t;

// The order and counts of the blocks:
// 1. COMM (1)
// 2. SONG (scount, 1-256)
// 3. INST (icount, 0-64)
// 4. WAVE (wcount, 0-64)


//
// Comment block (COMM), this block contains comment data for
// the module. There is always 1 COMM block in the module
//
constexpr BlockId BLOCK_ID_COMMENT     = 0x4d4d4f43; // "COMM"

//
// Song block (SONG), block contains a single song. The number of SONG blocks
// a module has is determined by the scount field in the header (1-256).
//
constexpr BlockId BLOCK_ID_SONG        = 0x474E4F53; // "SONG"

//
// Instrument block (INST), contains an instrument. The number of INST blocks
// the module has is determined by the icount field in the header (0-64).
//
constexpr BlockId BLOCK_ID_INSTRUMENT  = 0x54534e49; // "INST"

//
// Waveform block (WAVE), contains a waveform. The number of WAVE blocks a
// module has is determined by the wcount field in the header (0-64).
//
constexpr BlockId BLOCK_ID_WAVE        = 0x45564157; // "WAVE"

extern const char *FILE_SIGNATURE;

//
// Integer representing the current revision of the trackerboy file format.
// Starts at 0 (initial version) and is incremented on every change to the
// format.
//
static constexpr uint8_t FILE_REVISION = 0;

}
