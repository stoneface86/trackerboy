#pragma once

#include <vector>

#include "trackerboy/synth/HardwareFile.hpp"
#include "trackerboy/synth/Mixer.hpp"
#include "trackerboy/synth/Sequencer.hpp"
#include "trackerboy/ChType.hpp"


namespace trackerboy {

class Synth {
    
public:

    Synth(float samplingRate, float framerate = Gbs::FRAMERATE_GB) noexcept;

    float* buffer() noexcept;

    HardwareFile& hardware() noexcept;

    void fill(float buf[], size_t nsamples) noexcept;

    //
    // Reset the synthesizer by reseting all hardware components to defaults.
    //
    void reset() noexcept;
    
    uint8_t readRegister(uint16_t addr) const noexcept;

    // reinitialize channel
    // this is equivalent to writing bit 7 in NRx4 registers
    void restart(ChType ch) noexcept;

    //
    // Run the synth for 1 frame. Synthesized output is stored in
    // the synth's frame buffer. The number of samples generated is returned
    //
    size_t run() noexcept;

    
    void setFramerate(float framerate);

    void setOutputEnable(Gbs::OutputFlags flags) noexcept;
    void setOutputEnable(ChType ch, Gbs::Terminal terminal, bool enabled) noexcept;

    void setSamplingRate(float samplingRate);

    void writeRegister(uint16_t addr, uint8_t value) noexcept;

private:

    // output sampling rate
    float mSamplingRate;
    // interrupt rate of the gameboy VBlank interrupt
    float mFramerate;

    // Hardware components
    HardwareFile mHf;

    Mixer mMixer;
    Sequencer mSequencer;

    // number of cycles needed to execute to produce 1 sample
    // equal to the gameboy clock speed divided by the sampling rate
    float mCyclesPerSample;

    // number of cycles executed in 1 frame
    // equal to the gameboy clock speed divided by the framerate
    float mCyclesPerFrame;

    // fraction offset of cycles when determining frame length
    float mCycleOffset;

    // fractional offset of samples
    float mSampleOffset;

    // buffer of generated samples from the last run()
    std::vector<float> mFrameBuf;

    // channel panning settings
    // bits 7-4: Right panning enable for channels 1,2,3,4 (bit 4 = 1, ...)
    // bits 3-0: Left panning enable for channels 1,2,3,4 (bit 0 = 1, ...)
    uint8_t mOutputStat;

    // previous output from last run for each channel terminals
    uint8_t mChPrev[8];

    // used by fill()
    size_t mFillPos;
    size_t mLastFrameSize;

    void resizeFrameBuf();

    template <ChType ch>
    void updateOutput(int8_t &leftdelta, int8_t &rightdelta, uint32_t &fence) noexcept;

};

}
