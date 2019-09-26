
#include "trackerboy/synth.hpp"

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
    scf(Gbs::DEFAULT_SCF),
    stepSelection(Gbs::DEFAULT_STEP_COUNT),
    drf(Gbs::DEFAULT_DRF),
    lfsr(LFSR_INIT),
    shiftCounter(0),
    shiftCounterMax(calcCounterMax(drf, scf))
{
}

void NoiseChannel::reset() {
    EnvChannel::reset();
    shiftCounter = 0;
    lfsr = LFSR_INIT;
}

void NoiseChannel::setDrf(uint8_t _drf) {
    drf = _drf;
    shiftCounterMax = calcCounterMax(drf, scf);
}

void NoiseChannel::setNoise(uint8_t noiseReg) {
    drf = noiseReg & 0x7;
    stepSelection = static_cast<Gbs::NoiseSteps>((noiseReg >> 3) & 1);
    scf = noiseReg >> 4;
}

void NoiseChannel::setScf(uint8_t _scf) {
    if (_scf > Gbs::MAX_SCF) {
        _scf = Gbs::MAX_SCF;
    }
    scf = _scf;
    shiftCounterMax = calcCounterMax(drf, scf);
}

void NoiseChannel::setStepSelection(Gbs::NoiseSteps count) {
    stepSelection = count;
}

void NoiseChannel::step(unsigned cycles) {
    shiftCounter += cycles;
    unsigned shifts = shiftCounter / shiftCounterMax; // determine shifts to do
    shiftCounter %= shiftCounterMax; // adjust counter if overflow
    for (unsigned i = 0; i != shifts; ++i) {
        // xor bits 1 and 0 of the lfsr
        uint8_t xor = (lfsr & 0x1) ^ ((lfsr >> 1) & 0x1);
        // shift the register
        lfsr >>= 1;
        // set the resulting xor to bit 15 (feedback)
        lfsr |= xor << 14;
        if (stepSelection == Gbs::NOISE_STEPS_7) {
            // 7-bit lfsr, set bit 7 with the result
            lfsr &= ~0x40; // reset bit 7
            lfsr |= xor << 6; // set bit 7 result
        }
    }
    if (lfsr & 0x1) {
        // output is bit 0 inverted, so if bit 0 == 1, output MIN
        currentSample = Gbs::SAMPLE_MIN;
    } else {
        currentSample = Gbs::SAMPLE_MAX;
    }
}

}