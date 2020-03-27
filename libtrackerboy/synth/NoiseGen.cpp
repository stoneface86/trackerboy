
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
    mRegister(Gbs::DEFAULT_NOISE_REGISTER),
    mScf(Gbs::DEFAULT_SCF),
    mStepSelection(Gbs::DEFAULT_STEP_COUNT),
    mDrf(Gbs::DEFAULT_DRF),
    mLfsr(LFSR_INIT),
    mShiftCounter(0),
    mShiftCounterMax(calcCounterMax(mDrf, mScf))
{
}

void NoiseGen::restart() {
    mShiftCounter = 0;
    mLfsr = LFSR_INIT;
    mDrf = mRegister & 0x7;
    mStepSelection = static_cast<Gbs::NoiseSteps>((mRegister >> 3) & 1);
    mScf = mRegister >> 4;
    mShiftCounterMax = calcCounterMax(mDrf, mScf);
}

uint8_t NoiseGen::step(unsigned cycles) {
    mShiftCounter += cycles;
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

    // output is bit 0 inverted, so if bit 0 == 1, output MIN
    return (mLfsr & 0x1) ? Gbs::SAMPLE_MIN : Gbs::SAMPLE_MAX;
}

uint8_t NoiseGen::readRegister() {
    return mRegister;
}

void NoiseGen::writeRegister(uint8_t reg) {
    mRegister = reg;
}



}
