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

#include <cstdint>

namespace trackerboy {


class Generator {

public:

    //
    // Disables the generator. A disabled generator always outputs 0
    // Generators can only be re-enabled by restarting.
    //
    void disable() noexcept;

    //
    // Returns true if the generator is disabled, false otherwise.
    //
    bool disabled() const noexcept;

    //
    // Hardware reset the generator.
    //
    virtual void reset() noexcept = 0;

    //
    // Restart (retrigger) the generator. Counters are reset to 0. If the
    // generator was disabled, it is re-enabled.
    //
    virtual void restart() noexcept;

    //
    // Returns the fence, or the number of cycles to complete a period.
    //
    inline uint32_t fence() const noexcept {
        return (mFreqCounter > mPeriod) ? 0 : mPeriod - mFreqCounter;
    }

    //
    // Return the current output of the generator. Channels with an envelope
    // return 1 for the current envelope value and 0 for off.
    //
    inline uint8_t output() const noexcept {
        return mDisableMask & mOutput;
    }


protected:

    Generator(uint32_t defaultPeriod, uint8_t defaultOutput) noexcept;


    uint32_t mFreqCounter;
    uint32_t mPeriod;

    uint8_t mOutput;

private:

    static constexpr uint8_t ENABLED  = 0xFF;
    static constexpr uint8_t DISABLED = 0x00;

    uint8_t mDisableMask;

};



}
