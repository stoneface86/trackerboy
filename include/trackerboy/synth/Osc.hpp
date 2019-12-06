
#pragma once

#include <cstdint>

namespace trackerboy {

template <unsigned multiplier, size_t segments>
class Osc {

public:
    Osc(float samplingRate);

    void reset();

    void setFrequency(uint16_t frequency);

    void setWaveform(uint8_t waveform[segments / 2]);

    void generate(int16_t samples[], size_t nsamples);

private:

    // if speed is a concern, use fixed point instead of floats

    // samplingRate / gameboy clock rate
    float mFactor;

    // number of samples per waveform segment
    float mSamplesPerSegment;

    float mSampleCounter; // samples for the current segment to make
    
    uint8_t mCurrentSegment;
    // one byte in the waveform is two segments
    uint8_t mCurrentDoubleSegment;
    uint8_t mWaveform[segments / 2];
    uint8_t mWaveIndex;

    int16_t mPrevious;

};


using PulseOsc = Osc<4, 8>;
using WaveOsc = Osc<2, 32>;


}