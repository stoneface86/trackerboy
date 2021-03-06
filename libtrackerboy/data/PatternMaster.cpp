
#include "trackerboy/data/PatternMaster.hpp"

#include "internal/enumutils.hpp"

#include <stdexcept>

namespace trackerboy {


PatternMaster::PatternMaster(int rows) :
    mRows(rows)
{
    if (rows <= 0 || rows > MAX_ROWS) {
        throw std::invalid_argument("invalid row count");
    }
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

void PatternMaster::clear() {
    for (auto &trackMap : mMap) {
        trackMap.clear();
    }
}

int PatternMaster::rowSize() const noexcept {
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

Track const* PatternMaster::getTrack(ChType ch, uint8_t track) const {
    auto &chMap = mMap[static_cast<size_t>(ch)];
    auto iter = chMap.find(track);

    if (iter == chMap.end()) {
        return nullptr;
    } else {
        return &iter->second;
    }
}

void PatternMaster::remove(ChType ch, uint8_t track) {
    //uint16_t trackIndex = trackId(ch, track);
    auto &chMap = mMap[static_cast<size_t>(ch)];
    chMap.erase(track);
}

void PatternMaster::setRowSize(int newsize) {
    if (newsize <= 0 || newsize > MAX_ROWS) {
        throw std::invalid_argument("invalid row size given");
    }
    mRows = newsize;

    for (auto &chMap : mMap) {
        for (auto &iter : chMap) {
            iter.second.resize(mRows);
        }
    }
}

size_t PatternMaster::tracks(ChType ch) const noexcept {
    size_t count = 0;
    for (auto track : mMap[static_cast<size_t>(ch)]) {
        if (track.second.rowCount() != 0) {
            ++count;
        }
    }
    return count;
}

size_t PatternMaster::tracks() const noexcept {
    return tracks(ChType::ch1) + tracks(ChType::ch2) + tracks(ChType::ch3) + tracks(ChType::ch4);
    //return mMap[+ChType::ch1].size() +
    //       mMap[+ChType::ch2].size() +
    //       mMap[+ChType::ch3].size() +
    //       mMap[+ChType::ch4].size();
}

PatternMaster::Data::iterator PatternMaster::tracksBegin(ChType ch) {
    return mMap[static_cast<size_t>(ch)].begin();
}

PatternMaster::Data::const_iterator PatternMaster::tracksBegin(ChType ch) const {
    return mMap[static_cast<size_t>(ch)].begin();
}

PatternMaster::Data::iterator PatternMaster::tracksEnd(ChType ch) {
    return mMap[static_cast<size_t>(ch)].end();
}

PatternMaster::Data::const_iterator PatternMaster::tracksEnd(ChType ch) const {
    return mMap[static_cast<size_t>(ch)].end();
}


}
