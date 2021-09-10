
#include "trackerboy/data/PatternMap.hpp"

#include "internal/enumutils.hpp"

#include <stdexcept>

namespace trackerboy {


PatternMap::PatternMap(int rows) :
    mRows(rows)
{
    if (rows <= 0 || rows > MAX_ROWS) {
        throw std::invalid_argument("invalid row count");
    }
}

PatternMap::PatternMap(const PatternMap &master) :
    mRows(master.mRows),
    mMap{
        Data(master.mMap[0]),
        Data(master.mMap[1]),
        Data(master.mMap[2]),
        Data(master.mMap[3])
    }
{
}

void PatternMap::clear() {
    for (auto &trackMap : mMap) {
        trackMap.clear();
    }
}

int PatternMap::length() const noexcept {
    return mRows;
}


Pattern PatternMap::getPattern(OrderRow row) {
    return {
        getTrack(ChType::ch1, row[0]),
        getTrack(ChType::ch2, row[1]),
        getTrack(ChType::ch3, row[2]),
        getTrack(ChType::ch4, row[3])
    };
}

Track& PatternMap::getTrack(ChType ch, uint8_t track) {
    auto &chMap = mMap[static_cast<size_t>(ch)];
    auto iter = chMap.find(track);

    if (iter == chMap.end()) {
        // track does not exist, add it
        
        chMap.emplace(track, mRows);
        iter = chMap.find(track);
    }

    return iter->second;
}

Track const* PatternMap::getTrack(ChType ch, uint8_t track) const {
    auto &chMap = mMap[static_cast<size_t>(ch)];
    auto iter = chMap.find(track);

    if (iter == chMap.end()) {
        return nullptr;
    } else {
        return &iter->second;
    }
}

void PatternMap::remove(ChType ch, uint8_t track) {
    auto &chMap = mMap[static_cast<size_t>(ch)];
    chMap.erase(track);
}

void PatternMap::setLength(int newsize) {
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

size_t PatternMap::tracks(ChType ch) const noexcept {
    size_t count = 0;
    for (auto const& track : mMap[static_cast<size_t>(ch)]) {
        if (track.second.rowCount() != 0) {
            ++count;
        }
    }
    return count;
}

size_t PatternMap::tracks() const noexcept {
    return tracks(ChType::ch1) + tracks(ChType::ch2) + tracks(ChType::ch3) + tracks(ChType::ch4);
    //return mMap[+ChType::ch1].size() +
    //       mMap[+ChType::ch2].size() +
    //       mMap[+ChType::ch3].size() +
    //       mMap[+ChType::ch4].size();
}

PatternMap::Data::iterator PatternMap::tracksBegin(ChType ch) {
    return mMap[static_cast<size_t>(ch)].begin();
}

PatternMap::Data::const_iterator PatternMap::tracksBegin(ChType ch) const {
    return mMap[static_cast<size_t>(ch)].begin();
}

PatternMap::Data::iterator PatternMap::tracksEnd(ChType ch) {
    return mMap[static_cast<size_t>(ch)].end();
}

PatternMap::Data::const_iterator PatternMap::tracksEnd(ChType ch) const {
    return mMap[static_cast<size_t>(ch)].end();
}


}
