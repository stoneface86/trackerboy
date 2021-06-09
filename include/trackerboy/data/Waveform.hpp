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
#include "trackerboy/trackerboy.hpp"

#include <array>
#include <cstdint>

namespace trackerboy {

class Waveform : public DataItem {

public:

    using Data = std::array<uint8_t, GB_WAVERAM_SIZE>;

    Waveform() noexcept;

    Waveform(const Waveform &wave);

    Waveform(std::string const& hexstring);

    Data& data() noexcept;

    Data const& data() const noexcept;

    // convenience method, sets the waveform data from a string of hex nibbles
    void fromString(std::string const& hexstring);

    std::string toString() const;

    uint8_t& operator[](int index);

private:
    Data mData;
};

}
