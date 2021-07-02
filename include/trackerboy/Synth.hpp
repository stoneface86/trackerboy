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

#include "trackerboy/trackerboy.hpp"

#include "gbapu.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace trackerboy {

class Synth {
    
public:

    Synth(unsigned samplingRate, float framerate = GB_FRAMERATE_DMG) noexcept;
    ~Synth() = default;

    gbapu::Apu& apu() noexcept;

    unsigned samplerate() const noexcept;

    //
    // Returns the minimum number of samples per frame. Frame sizes alternate between
    // the result of this method and 1 extra sample.
    //
    size_t framesize() const noexcept;

    //
    // Reset the synthesizer by reseting all hardware components to defaults.
    //
    void reset() noexcept;

    //
    // Run the synth for 1 frame. Synthesized output is stored in
    // the apu's buffer. All samples must be read out before calling this method
    //
    void run() noexcept;

    //
    // Set the interval for 1 frame, default is DMG vblank or 59.7 Hz
    //
    void setFramerate(float framerate);

    void setSamplingRate(unsigned samplingRate);

    void setupBuffers();

private:

    gbapu::Apu mApu;
    
    // output sampling rate
    unsigned mSamplerate;
    // interrupt rate of the gameboy VBlank interrupt
    float mFramerate;

    // number of cycles executed in 1 frame
    // equal to the gameboy clock speed divided by the framerate
    float mCyclesPerFrame;

    // fraction offset of cycles leftover from the last run()
    float mCycleOffset;

    size_t mFrameSize;

    bool mResizeRequired;

};

}
