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

#include <cstdint>

namespace trackerboy {

class PulseGen : public Generator {

public:

    PulseGen() noexcept;

    Gbs::Duty duty() const noexcept;

    uint16_t frequency() const noexcept;

    void reset() noexcept override;

    //
    // Restart the generator, counters are reset to 0
    //
    void restart() noexcept override;

    //
    // Step the generator for the given number of cycles, returning the
    // current output (1 for output on, 0 for off)
    //
    void step(uint32_t cycles) noexcept;

    //
    // Set the duty of the pulse. Does not require restart.
    //
    void setDuty(Gbs::Duty duty) noexcept;

    //
    // Set the frequency of the output waveform. Does not require restart. If a sweep
    // is being applied to this generator, any changes will be lost on next sweep
    // trigger. (ie, changing this frequency does not modify the sweep's shadow
    // frequency).
    //
    void setFrequency(uint16_t frequency) noexcept;

private:

    uint16_t mFrequency;
    Gbs::Duty mDuty;
    
    unsigned mDutyCounter;


};



}
