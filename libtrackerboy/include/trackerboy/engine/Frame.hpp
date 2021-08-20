/*
** Trackerboy - Gameboy / Gameboy Color music tracker
** Copyright (C) 2019-2021 stoneface86
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

namespace trackerboy {

//
// Informational struct about the current frame being stepped.
//
struct Frame {

    constexpr Frame() :
        halted(true),
        startedNewRow(false),
        startedNewPattern(false),
        speed(0),
        time(0),
        order(0),
        row(0)
    {
    }

    bool halted;                // halt status
    bool startedNewRow;
    bool startedNewPattern;
    Speed speed;                // the current engine speed
    int time;                   // time index
    int order;                  // current order index
    int row;                    // current row index
};

}
