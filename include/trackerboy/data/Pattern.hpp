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

#include "trackerboy/data/Track.hpp"
#include "trackerboy/data/PatternRow.hpp"

namespace trackerboy {

//
// Utility class for accessing pattern data
//
class Pattern {

public:

    Pattern(Track &track1, Track &track2, Track &track3, Track &track4);

    //
    // Gets the row data at the given row index.
    //
    PatternRow operator[](int row);

    TrackRow& getTrackRow(ChType ch, int row);
    TrackRow const& getTrackRow(ChType ch, int row) const;

    //
    // Gets the pattern size, in rows.
    //
    int size() const;

    //
    // Returns the count of rows that will be stepped by the MusicRuntime. Track::rowCount
    // will be returned if there are no pattern skip effects in any of the track data
    //
    int totalRows();

private:
    Track *mTrack1;
    Track *mTrack2;
    Track *mTrack3;
    Track *mTrack4;

    // cache the calculated row count
    int mRowCount;
};



}
