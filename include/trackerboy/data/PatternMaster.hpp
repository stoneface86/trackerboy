/*
** Trackerboy - Gameboy / Gameboy Color music tracker
** Copyright (C) 2019-2020 stoneface86
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
*/

#pragma once

#include "trackerboy/ChType.hpp"
#include "trackerboy/data/Pattern.hpp"
#include "trackerboy/data/Track.hpp"
#include "trackerboy/data/TrackRow.hpp"

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

    PatternMaster(const PatternMaster &master);

    uint16_t rowSize();

    size_t tracks(ChType ch);
    size_t tracks();

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
