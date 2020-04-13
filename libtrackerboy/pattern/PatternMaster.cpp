
#include "trackerboy/pattern/PatternMaster.hpp"

namespace trackerboy {


namespace {

static inline uint16_t trackId(ChType ch, uint8_t track) {
    return track + (256 * static_cast<uint16_t>(ch));
}


}

PatternMaster::PatternMaster(uint16_t rows) :
    mRows(rows)
{
}

uint16_t PatternMaster::rowSize() {
    return mRows;
}


Pattern PatternMaster::getPattern(uint8_t track1, uint8_t track2, uint8_t track3, uint8_t track4) {
    return {
        getTrack(ChType::ch1, track1),
        getTrack(ChType::ch2, track2),
        getTrack(ChType::ch3, track3),
        getTrack(ChType::ch4, track4),
    };
}

Track PatternMaster::getTrack(ChType ch, uint8_t track) {
    uint16_t trackIndex = trackId(ch, track);
    auto iter = mMap.find(trackIndex);

    if (iter == mMap.end()) {
        // track does not exist, add it
        TrackData data(mRows);
        mMap[trackIndex].swap(data);
        iter = mMap.find(trackIndex);
    }

    return Track(iter->second.begin(), iter->second.end());
}

void PatternMaster::remove(ChType ch, uint8_t track) {
    uint16_t trackIndex = trackId(ch, track);

    mMap.erase(trackIndex);
}

void PatternMaster::setRowSize(uint16_t newsize) {
    // invalidates all Tracks return from getTrack!
    mRows = newsize;

    for (auto &iter : mMap) {
        iter.second.resize(mRows);
    }
}



}
