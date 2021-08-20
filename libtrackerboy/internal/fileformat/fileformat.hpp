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

#include "trackerboy/data/InfoStr.hpp"

#include <array>
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
// 24  +----------+----------+---------+-----------+
//     | m. rev   | n. rev   |  reserved           |
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
// 124 +-------------------------------------------+
//     | icount   | scount   | wcount   | system   |
// 128 +---------------------+----------+----------+
//     | customFramerate     |                     |
//     +---------------------+                     |
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

using Signature = std::array<char, 12>;

#pragma pack(push, 1)

//
// Header for major revision 0
//
struct Header0 {

    Signature signature;
    uint32_t versionMajor;
    uint32_t versionMinor;
    uint32_t versionPatch;
    uint8_t revision;
    uint8_t system; // DMG, SGB, custom
    uint16_t customFramerate; // framerate if system == System::custom

    InfoStr title;
    InfoStr artist;
    InfoStr copyright;
    uint16_t numberOfInstruments;
    uint16_t numberOfWaveforms;

    uint8_t reserved1[32];

};

//
// Header for major revision 1
//
struct Header1 {

    Signature signature;
    uint32_t versionMajor;
    uint32_t versionMinor;
    uint32_t versionPatch;
    uint8_t revMajor;
    uint8_t revMinor;
    uint16_t reserved;

    InfoStr title;
    InfoStr artist;
    InfoStr copyright;
    uint8_t icount;
    uint8_t scount;
    uint8_t wcount;
    uint8_t system; // DMG, SGB, custom
    uint16_t customFramerate; // framerate if system == System::custom
    uint8_t reserved1[30];

};

// current header
using HeaderCurrent = Header1;


union Header {
    Header0 rev0;
    Header1 rev1;
    HeaderCurrent current;
};

static_assert(sizeof(Header) == 160);

#pragma pack(pop)


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
constexpr BlockId BLOCK_ID_COMMENT     = 0x4D4D4F43; // "COMM"

//
// Song block (SONG), block contains a single song. The number of SONG blocks
// a module has is determined by the scount field in the header (1-256).
//
constexpr BlockId BLOCK_ID_SONG        = 0x474E4F53; // "SONG"

//
// Instrument block (INST), contains an instrument. The number of INST blocks
// the module has is determined by the icount field in the header (0-64).
//
constexpr BlockId BLOCK_ID_INSTRUMENT  = 0x54534E49; // "INST"

//
// Waveform block (WAVE), contains a waveform. The number of WAVE blocks a
// module has is determined by the wcount field in the header (0-64).
//
constexpr BlockId BLOCK_ID_WAVE        = 0x45564157; // "WAVE"

//
// Signature for modules - "\0TRACKERBOY\0"
//
extern Signature const FILE_SIGNATURE;

//
// Current major revision number of the file format. Incremented whenever a forward-compatibility
// breaking change is implemented in the file format. Changes such as a change to the layout
// of the payload or header.
//
static constexpr uint8_t FILE_REVISION_MAJOR = 1;

//
// Current minor revision number of the file format. Incremented whenever a non-breaking change
// is implemented in the format. Changes such as adding/removing extended commands to the
// payload or utilizing a reserved field in the header.
//
static constexpr uint8_t FILE_REVISION_MINOR = 0;

// most counter fields in the file format range from 1-256, but use a single byte for encoding
// the counter is biased by subtracting 1 such that 1...256 is represented by 0...255

// when writing: bias
// when reading: unbias

// returns the unbiased form of the given biased number
template <typename T>
constexpr inline T unbias(uint8_t num) {
    return (T)num + 1;
}

// return the biased form of the given number
// assumptions: num is an integral type whose value is between 1 and 256, inclusive.
template <typename T>
constexpr inline uint8_t bias(T num) {
    return (uint8_t)(num - 1);
}

//
// Upgrades the header to the current major version. true returned on success.
// Upgrading is done instead of having code to handle all major versions.
//
bool upgradeHeader(Header &header) noexcept;


// Revision history
//
// Rev B (1.0)
// Introduced in v0.5.0, adds multi-song support
//  - file revision is now a major/minor set of numbers
//  - removed numberOfInstruments and numberOfWaveforms fields in Header
//  - added icount, scount and wcount fields
//  - moved system and customFramerate fields after wcount
//  - remove INDX block from payload
//  - INST and WAVE blocks contain a single instrument and waveform, respectively
//  - Block data format now uses command sequences
//  - String encoding now specified, Header strings use ASCII, everything else uses UTF-8
//  - Payload is now terminated with a reversed signature - "\0YOBREKCART\0"
//
// Rev A (0.0)
// First initial version introduced in v0.2.0
//



}
