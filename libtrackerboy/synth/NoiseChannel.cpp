
#include "trackerboy/synth/NoiseChannel.hpp"

#define LFSR_INIT 0x7FFF
#define calcCounterMax(drf, scf) (DRF_TABLE[drf] << (scf+1))

namespace trackerboy {

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

NoiseChannel::NoiseChannel() : 
    EnvChannel(),
    mScf(Gbs::DEFAULT_SCF),
    mStepSelection(Gbs::DEFAULT_STEP_COUNT),
    mDrf(Gbs::DEFAULT_DRF),
    mLfsr(LFSR_INIT),
    mShiftCounter(0),
    mShiftCounterMax(calcCounterMax(mDrf, mScf))
{
}

void NoiseChannel::reset() {
    EnvChannel::reset();
    mShiftCounter = 0;
    mLfsr = LFSR_INIT;
}

void NoiseChannel::setDrf(uint8_t _drf) {
    mDrf = _drf;
    mShiftCounterMax = calcCounterMax(mDrf, mScf);
}

void NoiseChannel::setNoise(uint8_t noiseReg) {
    mDrf = noiseReg & 0x7;
    mStepSelection = static_cast<Gbs::NoiseSteps>((noiseReg >> 3) & 1);
    mScf = noiseReg >> 4;
}

void NoiseChannel::setScf(uint8_t _scf) {
    if (_scf > Gbs::MAX_SCF) {
        _scf = Gbs::MAX_SCF;
    }
    mScf = _scf;
    mShiftCounterMax = calcCounterMax(mDrf, mScf);
}

void NoiseChannel::setStepSelection(Gbs::NoiseSteps count) {
    mStepSelection = count;
}

void NoiseChannel::step(unsigned cycles) {
    mShiftCounter += cycles;
    unsigned shifts = mShiftCounter / mShiftCounterMax; // determine shifts to do
    mShiftCounter %= mShiftCounterMax; // adjust counter if overflow
    for (unsigned i = 0; i != shifts; ++i) {
        // xor bits 1 and 0 of the lfsr
        uint8_t xor = (mLfsr & 0x1) ^ ((mLfsr >> 1) & 0x1);
        // shift the register
        mLfsr >>= 1;
        // set the resulting xor to bit 15 (feedback)
        mLfsr |= xor << 14;
        if (mStepSelection == Gbs::NOISE_STEPS_7) {
            // 7-bit lfsr, set bit 7 with the result
            mLfsr &= ~0x40; // reset bit 7
            mLfsr |= xor << 6; // set bit 7 result
        }
    }
    if (mLfsr & 0x1) {
        // output is bit 0 inverted, so if bit 0 == 1, output MIN
        mCurrentSample = Gbs::SAMPLE_MIN;
    } else {
        mCurrentSample = Gbs::SAMPLE_MAX;
    }
}

}