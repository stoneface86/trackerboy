#pragma once

#include <vector>

#include "trackerboy/synth/HardwareFile.hpp"
#include "trackerboy/ChType.hpp"


namespace trackerboy {

class Synth {
    
public:

    Synth(float samplingRate);

    HardwareFile& hardware();

    void fill(float buf[], size_t nsamples);

    void setOutputEnable(Gbs::OutputFlags flags);
    void setOutputEnable(ChType ch, Gbs::Terminal terminal, bool enabled);

private:

    enum TriggerType {
        NONE,
        SWEEP,
        ENV
    };

    // three triggers, two for sweep and one for envelope
    static constexpr size_t TRIGGER_COUNT = 3;
    // trigger sequence
    static TriggerType const TRIGGER_SEQUENCE[TRIGGER_COUNT];



    float mSamplingRate;

    HardwareFile mHf;

    // number of cycles needed to execute to produce 1 sample
    // equal to the gameboy clock speed divided by the sampling rate
    float mCyclesPerSample;

    // sample times between each trigger
    float mTriggerTimings[3];

    float mSampleCounter;
    float mSamplesToTrigger;
    unsigned mTriggerIndex;

    // input buffer for generating samples, size is the largest trigger timing
    std::vector<float> mInputBuffer;

    uint8_t mOutputStat;

    // methods

    template <ChType ch>
    void run(float inbuf[], float out[], size_t nsamples);

};

}
