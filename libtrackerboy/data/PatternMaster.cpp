
#include "trackerboy/data/PatternMaster.hpp"

namespace trackerboy {


PatternMaster::PatternMaster(uint16_t rows) :
    mRows(rows)
{
}

PatternMaster::PatternMaster(const PatternMaster &master) :
    mRows(master.mRows),
    mMap{
        Data(master.mMap[0]),
        Data(master.mMap[1]),
        Data(master.mMap[2]),
        Data(master.mMap[3])
    }
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
        getTrack(ChType::ch4, track4)
    };
}

Track& PatternMaster::getTrack(ChType ch, uint8_t track) {
    auto &chMap = mMap[static_cast<size_t>(ch)];
    auto iter = chMap.find(track);

    if (iter == chMap.end()) {
        // track does not exist, add it
        //TrackData data(mRows);
        
        chMap.emplace(track, mRows);
        iter = chMap.find(track);
    }

    return iter->second;
    //return Track(iter->second.begin(), iter->second.end());
}

void PatternMaster::remove(ChType ch, uint8_t track) {
    //uint16_t trackIndex = trackId(ch, track);
    auto &chMap = mMap[static_cast<size_t>(ch)];
    chMap.erase(track);
}

void PatternMaster::setRowSize(uint16_t newsize) {
    mRows = newsize;

    for (auto &chMap : mMap) {
        for (auto &iter : chMap) {
            iter.second.resize(mRows);
        }
    }
}

size_t PatternMaster::tracks(ChType ch) {
    return mMap[static_cast<size_t>(ch)].size();
}

PatternMaster::Data::iterator PatternMaster::tracksBegin(ChType ch) {
    return mMap[static_cast<size_t>(ch)].begin();
}

PatternMaster::Data::iterator PatternMaster::tracksEnd(ChType ch) {
    return mMap[static_cast<size_t>(ch)].end();
}


}
