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
#include "trackerboy/data/PatternMaster.hpp"
#include "trackerboy/fileformat.hpp"
#include "trackerboy/gbs.hpp"
#include "trackerboy/Speed.hpp"

#include <fstream>

namespace trackerboy {


class Song : public DataItem {

public:

    enum class Mode : uint8_t {
        speed,              // use the speed setting
        speedFromTempo      // determine speed from tempo setting
    };

    static constexpr uint8_t DEFAULT_RPB = 4;
    static constexpr uint16_t DEFAULT_TEMPO = 150;
    static constexpr Mode DEFAULT_MODE = Mode::speedFromTempo;
    // Tempo = 150, RPB = 4  => 6.0 frames per row
    static constexpr Speed DEFAULT_SPEED = 0x30;

    Song();
    Song(const Song &song);

    ~Song();

    uint8_t rowsPerBeat();

    Mode mode();

    uint16_t tempo();

    Speed speed();

    std::vector<Order>& orders();

    PatternMaster& patterns();

    Pattern getPattern(uint8_t orderNo);

    TrackRow getRow(ChType ch, uint8_t order, uint16_t row);

    void setRowsPerBeat(uint8_t rowsPerBeat);

    void setTempo(uint16_t tempo);

    void setMode(Mode mode);

    void setSpeed(Speed speed);

    // apply the current mode to tempo/speed
    void apply(float framerate = Gbs::FRAMERATE_GB);

protected:

    virtual FormatError deserializeData(std::istream &stream) noexcept override;
    
    virtual FormatError serializeData(std::ostream &stream) noexcept override;


private:

    //void calcSpeed();

    PatternMaster mMaster;
    std::vector<Order> mOrder;

    uint8_t mRowsPerBeat;
    uint16_t mTempo;
    Mode mMode;

    // Speed - fixed point Q5.3
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
