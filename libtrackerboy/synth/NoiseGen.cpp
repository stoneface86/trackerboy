
#pragma once

#include "trackerboy/synth/NoiseGen.hpp"

#include <cmath>

constexpr uint16_t LFSR_INIT = 0x7FFF;
#define calcCounterMax(drf, scf) (DRF_TABLE[drf] << (scf+1))

namespace {

static const uint8_t DRF_TABLE[] = {
    8,
    16,
    32,
    48,
    64,
    80,
    96,
    112
};

}

namespace trackerboy {

NoiseGen::NoiseGen() :
    mScf(Gbs::DEFAULT_SCF),
    mStepSelection(Gbs::DEFAULT_STEP_COUNT),
    mDrf(Gbs::DEFAULT_DRF),
    mLfsr(LFSR_INIT),
    mShiftCounter(0),
    mShiftCounterMax(calcCounterMax(mDrf, mScf)),
    mDrift(0.0f)
{
}


void NoiseGen::generate(float buf[], size_t nsamples, float cps) {
    for (size_t i = 0; i != nsamples; ++i) {
        // determine the number of cylces to run for this sample
        float cycles = cps + mDrift;
        unsigned cyclesWhole = static_cast<unsigned>(cycles);
        // drift is the fractional part of the total cycles
        mDrift = cycles - cyclesWhole;
        // update the shift counter, determine how many shifts are needed
        mShiftCounter += cyclesWhole;

        shift();

        if (mLfsr & 0x1) {
            // output is bit 0 inverted, so if bit 0 == 1, output MIN
            *buf++ = -1.0f;
        } else {
            *buf++ = 1.0f;
        }


    }
}

void NoiseGen::reset() {
    mShiftCounter = 0;
    mDrift = 0.0f;
    mLfsr = LFSR_INIT;
}

void NoiseGen::run(size_t nsamples, float cps) {
    // this might overflow
    float cycles = (nsamples * cps) + mDrift;
    unsigned cyclesWhole = static_cast<unsigned>(cycles);
    mDrift = cycles - cyclesWhole;
    mShiftCounter += cyclesWhole;
    shift();
}

void NoiseGen::setNoise(uint8_t noiseReg) {
    mDrf = noiseReg & 0x7;
    mStepSelection = static_cast<Gbs::NoiseSteps>((noiseReg >> 3) & 1);
    mScf = noiseReg >> 4;
    mShiftCounterMax = calcCounterMax(mDrf, mScf);
}

void NoiseGen::shift() {
    while (mShiftCounter >= mShiftCounterMax) {
        mShiftCounter -= mShiftCounterMax;

        // xor bits 1 and 0 of the lfsr
        uint8_t result = (mLfsr & 0x1) ^ ((mLfsr >> 1) & 0x1);
        // shift the register
        mLfsr >>= 1;
        // set the resulting xor to bit 15 (feedback)
        mLfsr |= result << 14;
        if (mStepSelection == Gbs::NOISE_STEPS_7) {
            // 7-bit lfsr, set bit 7 with the result
            mLfsr &= ~0x40; // reset bit 7
            mLfsr |= result << 6; // set bit 7 result
        }
    }
}


}