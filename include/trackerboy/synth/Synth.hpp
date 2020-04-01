#pragma once

#include <vector>

#include "trackerboy/synth/HardwareFile.hpp"
#include "trackerboy/synth/Mixer.hpp"
#include "trackerboy/synth/Sequencer.hpp"
#include "trackerboy/ChType.hpp"


namespace trackerboy {

class Synth {
    
public:

    Synth(float samplingRate, float framerate = Gbs::FRAMERATE_GB);

    float* buffer();

    HardwareFile& hardware();

    
    void fill(float buf[], size_t nsamples);

    //
    // Reset the synthesizer by reseting all hardware components to defaults.
    //
    void reset();
    
    uint8_t readRegister(uint16_t addr);

    // reinitialize channel
    // this is equivalent to writing bit 7 in NRx4 registers
    void restart(ChType ch);

    //
    // Run the synth for 1 frame. Synthesized output is stored in
    // the synth's frame buffer. The number of samples generated is returned
    //
    size_t run();

    void setOutputEnable(Gbs::OutputFlags flags);
    void setOutputEnable(ChType ch, Gbs::Terminal terminal, bool enabled);

    void writeRegister(uint16_t addr, uint8_t value);

private:




    float mSamplingRate;
    float mFramerate;

    HardwareFile mHf;

    Mixer mMixer;
    Sequencer mSequencer;

    // number of cycles needed to execute to produce 1 sample
    // equal to the gameboy clock speed divided by the sampling rate
    float mCyclesPerSample;

    float mCyclesPerFrame;

    // fraction offset of cycles when determining frame length
    float mCycleOffset;

    // fractional offset of samples
    float mSampleOffset;

    std::vector<float> mFrameBuf;

    uint8_t mOutputStat;

    uint8_t mChPrev[8];

    size_t mFillPos;
    size_t mLastFrameSize;

    template <ChType ch>
    void updateOutput(int8_t &leftdelta, int8_t &rightdelta, uint32_t &fence);

};

}
