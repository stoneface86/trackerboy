
#include "trackerboy/engine/PatternCursor.hpp"


namespace trackerboy {


PatternCursor::PatternCursor() {

}

template <ChType ch>
TrackRow& PatternCursor::get() {
    return *(mIters[static_cast<int>(ch)]);
}

bool PatternCursor::next() {
    if (++(mIters[0]) == mEnd) {
        return true;
    } else {
        ++(mIters[1]);
        ++(mIters[2]);
        ++(mIters[3]);
        return false;
    }
}

void PatternCursor::setPattern(Pattern &&pattern, uint8_t startingRow) {
    auto &track1 = std::get<0>(pattern);
    mIters[0] = track1.begin() + startingRow;
    mEnd = track1.end();

    mIters[1] = std::get<1>(pattern).begin() + startingRow;
    mIters[2] = std::get<2>(pattern).begin() + startingRow;
    mIters[3] = std::get<3>(pattern).begin() + startingRow;
}



template TrackRow& PatternCursor::get<ChType::ch1>();
template TrackRow& PatternCursor::get<ChType::ch2>();
template TrackRow& PatternCursor::get<ChType::ch3>();
template TrackRow& PatternCursor::get<ChType::ch4>();



}
