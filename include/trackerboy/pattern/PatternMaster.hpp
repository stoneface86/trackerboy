
#pragma once

#include "trackerboy/pattern/Pattern.hpp"
#include "trackerboy/pattern/TrackData.hpp"
#include "trackerboy/pattern/TrackRow.hpp"
#include "trackerboy/pattern/Track.hpp"

#include <cstdint>
#include <unordered_map>

namespace trackerboy {


//
// Container class for all patterns for a song
//
class PatternMaster {

public:

    static constexpr uint16_t MAX_ROWS = 256;

    PatternMaster(uint16_t mRows);

    uint16_t rowSize();

    //
    // Utility method. Calls getTrack for all 4 channels and stores each track
    // into a Pattern struct.
    //
    Pattern getPattern(uint8_t track1, uint8_t track2, uint8_t track3, uint8_t track4);

    // Get the track from the given channel with the given track id. If the track does
    // not exist yet, it will be created.
    Track getTrack(ChType ch, uint8_t track);

    // Removes a given track from the master
    void remove(ChType ch, uint8_t track);

    void setRowSize(uint16_t newsize);


private:

    uint16_t mRows;

    // maps a track id -> TrackData
    // the map index is the track id offset by a multiple of 256
    // id 0-255     (CH1 id + (256 * 0))
    //    256-511   (CH2 id + (256 * 1))
    //    512-767   (CH3 id + (256 * 2))
    //    768-1023  (CH4 id + (256 * 3))
    //    
    std::unordered_map<uint16_t, TrackData> mMap;


};




}
