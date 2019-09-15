
#pragma once

#include <array>
#include <cstdint>
using std::uint8_t;

#include "note.hpp"


namespace trackerboy {




struct TrackRow {
    Note note;
    uint8_t instrumentId;
    //effect
};

typedef TrackRow PatternRow[4];


class Pattern {

    std::array<PatternRow, 64> table;


};



}