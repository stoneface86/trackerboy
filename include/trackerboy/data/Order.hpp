
#pragma once

#include <cstdint>

namespace trackerboy {

#pragma pack(push, 1)

// An order is an assignment of track Ids for each channel. A sequence of these
// determines the layout of a song.
struct Order {

    // same as Pattern, except with IDs instead of iterators

    uint8_t track1Id;
    uint8_t track2Id;
    uint8_t track3Id;
    uint8_t track4Id;


};

#pragma pack(pop)



}
