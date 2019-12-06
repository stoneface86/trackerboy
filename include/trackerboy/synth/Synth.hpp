#pragma once

#include "ChannelFile.hpp"
#include "Mixer.hpp"
#include "Sequencer.hpp"


namespace trackerboy {

class Synth {
    ChannelFile mCf;
    Mixer mMixer;
    Sequencer mSequencer;

    float mSamplingRate;
    // fixed point Q7.25
    uint32_t mStepsPerSample;
    uint32_t mStepCounter;
    

public:

    Synth(float samplingRate);

    ChannelFile& getChannels();
    Mixer& getMixer();
    Sequencer& getSequencer();

    void setQuality(float quality);

    void step(int16_t &left, int16_t &right);

    void fill(int16_t leftBuf[], int16_t rightBuf[], size_t nsamples);

    void fill(int16_t buf[], size_t nsamples);
};

}