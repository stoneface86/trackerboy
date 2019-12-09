
#pragma once

#include <cstdint>
#include <vector>

namespace trackerboy {

class Osc {

public:

    Osc(float samplingRate, size_t multiplier, size_t waveformSize);

    void reset();

    void setFrequency(uint16_t frequency);

    void generate(int16_t samples[], size_t nsamples);
 
protected:
    struct Delta {
        int16_t change; // the change in volume
        float time;     // time, in samples, when this delta occurs
        float duration; // duration of the transition, in samples
    };

    // waveform as amplitude changes (deltas)
    // any nonzero value indicates a transition, which will have a bandlimited step
    std::vector<Delta> mDeltaBuf;

private:

    void fillPeriod();

    const size_t mWaveformSize;
    const size_t mMultiplier;

    // samplingRate / gameboy clock rate
    float mFactor;

    // number of samples per waveform transition (delta)
    float mSamplesPerDelta;

    // mSamplesPerDelta * mWaveformSize
    float mSamplesPerPeriod;

    // counter used for timing purposes
    float mSampleCounter;


    int16_t mPrevious; // last sample generated

    // set by fillPeriod()
    std::vector<int16_t> mPeriodBuf;
    bool mPeriodReady;



};


}