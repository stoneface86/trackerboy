
#pragma once

#include "trackerboy/ChType.hpp"
#include "trackerboy/data/Pattern.hpp"
#include "trackerboy/data/TrackRow.hpp"
#include "trackerboy/data/Track.hpp"

#include <cstdint>
#include <unordered_map>

namespace trackerboy {


//
// Container class for all patterns for a song
//
class PatternMaster {

public:

    using Data = std::unordered_map<uint8_t, Track>;

    static constexpr uint16_t MAX_ROWS = 256;

    PatternMaster(uint16_t mRows);

    uint16_t rowSize();

    Data::iterator tracksBegin(ChType ch);

    Data::iterator tracksEnd(ChType ch);

    //
    // Utility method. Calls getTrack for all 4 channels and stores each track
    // into a Pattern struct.
    //
    Pattern getPattern(uint8_t track1, uint8_t track2, uint8_t track3, uint8_t track4);

    // Get the track from the given channel with the given track id. If the track does
    // not exist yet, it will be created.
    Track& getTrack(ChType ch, uint8_t track);

    

    // Removes a given track from the master
    void remove(ChType ch, uint8_t track);

    void setRowSize(uint16_t newsize);


private:

    uint16_t mRows;

    // maps a track id -> TrackData
        
    Data mMap[4];


};




}
