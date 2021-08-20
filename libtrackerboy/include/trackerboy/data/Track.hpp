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

#include "trackerboy/data/TrackRow.hpp"

#include <vector>

namespace trackerboy {


// container class for track data

class Track {

public:

    using Data = std::vector<TrackRow>;

    Track(int rows);

    TrackRow& operator[](int row);
    TrackRow const& operator[](int row) const;

    Data::iterator begin();
    Data::const_iterator begin() const;

    void clear(int rowStart, int rowEnd);

    void clearEffect(int row, int effectNo);

    void clearInstrument(int row);

    void clearNote(int row);

    Data::iterator end();
    Data::const_iterator end() const;

    void setEffect(int row, int effectNo, EffectType effect, uint8_t param = 0);

    void setInstrument(int row, uint8_t instrumentId);

    void setNote(int row, uint8_t note);

    void replace(int rowno, TrackRow &row);

    void resize(int newSize);

    int rowCount() const;

    int size() const;

private:

    Data mData;

};



}
