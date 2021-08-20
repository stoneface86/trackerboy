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

#include "trackerboy/data/DataItem.hpp"
#include "trackerboy/data/Order.hpp"
#include "trackerboy/data/OrderRow.hpp"
#include "trackerboy/data/PatternMaster.hpp"
#include "trackerboy/trackerboy.hpp"

namespace trackerboy {


class Song : public DataItem {

public:


    static constexpr int DEFAULT_RPB = 4;
    static constexpr int DEFAULT_RPM = 16;
    // Tempo = 150, RPB = 4  => 6.0 frames per row
    static constexpr Speed DEFAULT_SPEED = 0x60;
    static constexpr uint16_t DEFAULT_ROWS = 64;

    Song();
    Song(const Song &song);

    ~Song();

    void reset() noexcept;

    int rowsPerBeat() const noexcept;

    int rowsPerMeasure() const noexcept;

    Speed speed() const noexcept;

    Order& order() noexcept;
    Order const& order() const noexcept;

    PatternMaster& patterns() noexcept;
    PatternMaster const& patterns() const noexcept;

    Pattern getPattern(int orderNo);

    TrackRow& getRow(ChType ch, int order, int row);
    TrackRow getRow(ChType ch, int order, int row) const;

    void setRowsPerBeat(int rowsPerBeat);

    void setRowsPerMeasure(int rowsPerMeasure);

    void setSpeed(Speed speed);

    //
    // Calculates a speed setting from the given tempo and framerate. The
    // estimated speed is rounded and clamped to the minimum and maximum values.
    //
    Speed estimateSpeed(float tempo, float framerate = GB_FRAMERATE_DMG) const noexcept;

    float tempo(float framerate = GB_FRAMERATE_DMG) const noexcept;

private:

    //void calcSpeed();

    PatternMaster mMaster;
    Order mOrder;

    int mRowsPerBeat;
    int mRowsPerMeasure;

    // Speed - fixed point Q4.4
    // frame timing for each row
    // 4.0:   4, 4, 4, 4, ...
    // 4.125: 4, 4, 4, 4, 4, 4, 4, 5, ...
    // 4.25:  4, 4, 4, 5, 4, 4, 4, 5, ...
    // 4.375: 4, 4, 5, 4, 4, 5, 4, 4, 5, ...
    // 4.5:   4, 5, 4, 5, ...
    // 4.675: 4, 5, 5, 4, 5, 5, 4, 5, 5, 4, ...
    // 4.75:  4, 5, 5, 5, 4, 5, 5, 5, 4, ...
    // 4.875: 4, 5, 5, 5, 5, 5, 5, 4, ...

    Speed mSpeed; // frames per row

};


}
