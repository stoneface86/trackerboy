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
#include "trackerboy/data/DataItem.hpp"

#include <cstdint>

namespace trackerboy {


// Instruments are just a combination of effect settings that get
// applied on note trigger, used as a convenience for the composer




class Instrument : public DataItem {

public:

    #pragma pack(push, 1)
    struct Data {
        uint8_t channel;        // channel id, used by the ui
        uint8_t timbre;         // V0x effect
        uint8_t envelope;       // Exx
        uint8_t panning;        // Ixy
        uint8_t delay;          // 0 for no delay (Gxx)
        uint8_t duration;       // 0 for infinite duration (Sxx)
        int8_t tune;            // Pxx
        uint8_t vibrato;        // 4xy
        uint8_t vibratoDelay;   // 5xx
    };
    #pragma pack(pop)

    Instrument();

    Instrument(const Instrument &instrument); // copy constructor

    Data& data();

private:
    Data mData;
};


}
