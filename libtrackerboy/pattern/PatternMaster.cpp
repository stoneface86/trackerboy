
#include "trackerboy/pattern/PatternMaster.hpp"


namespace trackerboy {


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
    TrackMaster &tm = mTrackMasters[static_cast<size_t>(ch)];
    auto iter = tm.map.find(track);
    
    size_t offset;

    if (iter == tm.map.end()) {
        // allocate the track since it does not exist
        offset = tm.data.size();
        tm.data.resize(tm.data.size() + MAX_ROWS);
        tm.map[track] = offset / MAX_ROWS;

    } else {
        offset = iter->second * MAX_ROWS;
    }

    auto dataIter = tm.data.begin() + offset;
    return Track(dataIter, dataIter + mRows);
}

void PatternMaster::setRowSize(uint16_t newsize) {
    // invalidates all Tracks return from getTrack!
    mRows = newsize;
}


}
