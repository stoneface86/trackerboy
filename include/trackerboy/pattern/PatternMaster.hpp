
#pragma once

#include "trackerboy/pattern/Pattern.hpp"
#include "trackerboy/pattern/TrackData.hpp"
#include "trackerboy/pattern/TrackRow.hpp"
#include "trackerboy/pattern/Track.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace trackerboy {


//
// Container class for all patterns for a song
//
class PatternMaster {

public:

    static constexpr uint16_t MAX_ROWS = 256;

    PatternMaster(uint16_t mRows);

    uint16_t rowSize();

    Pattern getPattern(uint8_t track1, uint8_t track2, uint8_t track3, uint8_t track4);

    // Get the track from the given channel with the given track id. If the track does
    // not exist yet, it will be created.
    Track getTrack(ChType ch, uint8_t track);

    // Removes a given track from the master
    void remove(ChType ch, uint8_t track);

    void setRowSize(uint16_t newsize);


private:

    struct TrackMaster {
        TrackData data;
        std::unordered_map<uint8_t, size_t> map;
    };

    uint16_t mRows;

    TrackMaster mTrackMasters[4];

};




}
