#pragma once

#include <fstream>
#include <vector>

#include "trackerboy/ChType.hpp"
#include "trackerboy/EffectType.hpp"
#include "trackerboy/pattern/Track.hpp"
#include "trackerboy/pattern/TrackRow.hpp"


namespace trackerboy {


class Pattern {

public:

    using PatternData = std::vector<TrackRow>;
    using Iterator = PatternData::const_iterator;

    static constexpr size_t MAX_ROWS = 256;
    static constexpr size_t DEFAULT_ROWS = 64;

    Pattern(size_t nrows = DEFAULT_ROWS);

    Iterator begin();

    void clear(uint8_t rowStart, uint8_t rowEnd);

    TrackRow* data();

    Iterator end();

    void setSize(size_t nrows);

    size_t size();

    Track track(ChType id);

private:
    size_t mSize;
    PatternData mData;

};



}
