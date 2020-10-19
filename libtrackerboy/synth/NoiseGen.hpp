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

#include "synth/Generator.hpp"
#include "trackerboy/gbs.hpp"

namespace trackerboy {

class NoiseGen : public Generator {

public:

    NoiseGen() noexcept;

    void reset() noexcept override;

    //
    // channel retrigger. LFSR is re-initialized, counters are reset
    // and the period is reloaded from the set register
    //
    void restart() noexcept override;

    //
    // Step the generator for the given number of cycles, returning the
    // current output.
    //
    void step(uint32_t cycles) noexcept;

    //
    // Write the given value to this generator's register, NR43
    //
    void writeRegister(uint8_t reg) noexcept;

    //
    // Returns the contents of this generator's register
    //
    uint8_t readRegister() const noexcept;

private:

    // NR43 register contents
    uint8_t mRegister;

    // width of the LFSR (15-bit or 7-bit)
    Gbs::NoiseSteps mStepSelection;
    // lfsr: linear feedback shift register
    uint16_t mLfsr;


};



}
