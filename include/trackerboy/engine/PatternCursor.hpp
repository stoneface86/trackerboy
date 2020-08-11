#pragma once

#include "trackerboy/data/Pattern.hpp"
#include "trackerboy/ChType.hpp"

namespace trackerboy {


class PatternCursor {

public:

    PatternCursor();

    template <ChType ch>
    TrackRow& get();

    bool next();

    void setPattern(Pattern &&pattern, uint8_t startingRow = 0);

private:

    Track::Data::iterator mIters[4];
    Track::Data::iterator mEnd;


};

}
