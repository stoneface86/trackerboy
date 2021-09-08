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

#include "trackerboy/trackerboy.hpp"
#include "trackerboy/data/Pattern.hpp"
#include "trackerboy/data/Track.hpp"
#include "trackerboy/data/OrderRow.hpp"

#include <array>
#include <cstdint>
#include <unordered_map>

namespace trackerboy {


//
// Container class for all patterns for a song. Pattern data is stored
// in Track instances, which are mapped by a ChType and uint8_t track id
//
class PatternMap {

public:

    using Data = std::unordered_map<uint8_t, Track>;

    static constexpr int MAX_ROWS = 256;

    PatternMap(int rows);

    PatternMap(const PatternMap &master);

    void clear();

    //
    // Gets the length in rows, of all patterns in this map
    //
    int length() const noexcept;

    size_t tracks(ChType ch) const noexcept;
    size_t tracks() const noexcept;

    Data::iterator tracksBegin(ChType ch);
    Data::const_iterator tracksBegin(ChType ch) const;

    Data::iterator tracksEnd(ChType ch);
    Data::const_iterator tracksEnd(ChType ch) const;

    //
    // Utility method. Calls getTrack for all 4 channels and stores each track
    // into a Pattern struct.
    //
    Pattern getPattern(OrderRow row);

    // Get the track from the given channel with the given track id. If the track does
    // not exist yet, it will be created.
    Track& getTrack(ChType ch, uint8_t track);

    //
    // similiar to getTrack, but returns nullptr if the track does not exist
    //
    Track const* getTrack(ChType ch, uint8_t track) const;

    // Removes a given track from the master
    void remove(ChType ch, uint8_t track);

    //
    // Sets the length, in rows, of all patterns in this map. Note that this is a
    // destructive action if newsize < length().
    //
    void setLength(int newsize);


private:

    int mRows;

    // maps a track id -> TrackData
        
    std::array<Data, 4> mMap;


};




}
