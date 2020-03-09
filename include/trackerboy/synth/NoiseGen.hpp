
#pragma once

#include "trackerboy/gbs.hpp"


namespace trackerboy {

class NoiseGen {

public:

    NoiseGen();

    void generate(float buf[], size_t nsamples, float cps);

    void reset();

    void run(size_t nsamples, float cps);

    void setNoise(uint8_t reg);


private:

    // shift the lfsr as needed
    void shift();

    // scf: Shift clock frequency
    uint8_t mScf;
    // width of the LFSR (15-bit or 7-bit)
    Gbs::NoiseSteps mStepSelection;
    // drf: Dividing ratio frequency
    uint8_t mDrf;
    // lfsr: linear feedback shift register
    uint16_t mLfsr;
    unsigned mShiftCounter;
    unsigned mShiftCounterMax;
    // the fractional part that is truncated when generating samples
    float mDrift;
};



}