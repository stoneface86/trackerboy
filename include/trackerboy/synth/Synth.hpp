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

#include "trackerboy/synth/HardwareFile.hpp"
//#include "trackerboy/synth/Mixer.hpp"
#include "trackerboy/synth/Sequencer.hpp"
#include "trackerboy/ChType.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace trackerboy {

class Synth {
    
public:

    Synth(unsigned samplingRate, float framerate = Gbs::FRAMERATE_GB) noexcept;
    ~Synth();


    //
    // Get the frame buffer
    //
    int16_t* buffer() noexcept;

    HardwareFile& hardware() noexcept;

    //
    // Reset the synthesizer by reseting all hardware components to defaults.
    //
    void reset() noexcept;
    
    uint8_t readRegister(uint16_t addr) noexcept;

    // reinitialize channel
    // this is equivalent to writing bit 7 in NRx4 registers
    void restart(ChType ch) noexcept;

    //
    // Run the synth for 1 frame. Synthesized output is stored in
    // the synth's frame buffer. The number of samples generated is returned
    //
    size_t run() noexcept;
    
    //
    // Set the interval for 1 frame, default is DMG vblank or 59.7 Hz
    //
    void setFramerate(float framerate);

    // util method TODO: move this to ChannelControl
    void setFrequency(ChType ch, uint16_t freq);

    void setOutputEnable(Gbs::OutputFlags flags) noexcept;
    void setOutputEnable(ChType ch, Gbs::Terminal terminal, bool enabled) noexcept;

    void setSamplingRate(unsigned samplingRate);

    void setupBuffers();

    void step(uint32_t cycles) noexcept;

    void writeRegister(uint16_t addr, uint8_t value) noexcept;

private:

    // PIMPL idiom
    struct Internal;
    std::unique_ptr<Internal> mInternal;

    // output sampling rate
    unsigned mSamplerate;
    // interrupt rate of the gameboy VBlank interrupt
    float mFramerate;

    // Hardware components
    HardwareFile mHf;
    Sequencer mSequencer;

    // number of cycles executed in 1 frame
    // equal to the gameboy clock speed divided by the framerate
    float mCyclesPerFrame;

    // fraction offset of cycles when determining frame length
    float mCycleOffset;

    // buffer of generated samples from the last run()
    std::vector<int16_t> mFrameBuf;

    // channel panning settings
    // bits 7-4: Right panning enable for channels 1,2,3,4 (bit 4 = 1, ...)
    // bits 3-0: Left panning enable for channels 1,2,3,4 (bit 0 = 1, ...)
    uint8_t mOutputStat;

    // previous output from last run for each channel terminals
    int8_t mChPrev[8];

    size_t mLastFrameSize;

    // cycles til the next fence
    uint32_t mFence;
    // current time offset in cycles
    uint32_t mCycletime;

    //void resizeFrameBuf();

    template <ChType ch>
    void updateOutput(int8_t &leftdelta, int8_t &rightdelta, uint32_t &fence) noexcept;

};

}
