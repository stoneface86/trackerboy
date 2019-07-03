
#include "gbsynth.h"

#define LFSR_INIT 0x7FFF
#define calcCounterMax(drf, scf) (DRF_TABLE[drf] << (scf+1))

namespace gbsynth {

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
        scf(DEFAULT_SCF),
        stepSelection((StepCount)DEFAULT_STEP_COUNT),
        drf(DEFAULT_DRF),
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

    void NoiseChannel::setScf(uint8_t scf) {
        if (scf > MAX_SCF) {
            scf = MAX_SCF;
        }
        this->scf = scf;
        shiftCounterMax = calcCounterMax(drf, scf);
    }

    void NoiseChannel::setStepSelection(StepCount count) {
        stepSelection = count;
    }

    void NoiseChannel::setDrf(uint8_t drf) {
        this->drf = drf;
        shiftCounterMax = calcCounterMax(drf, scf);
    }

    uint8_t NoiseChannel::generate(unsigned cycles) {
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
            if (stepSelection == STEPS_7) {
                // 7-bit lfsr, set bit 7 with the result
                lfsr &= ~0x40; // reset bit 7
                lfsr |= xor << 6; // set bit 7 result
            }
        }
        if (lfsr & 0x1) {
            // output is bit 0 inverted, so if bit 0 == 1, output MIN
            return SAMPLE_MIN;
        } else {
            return SAMPLE_MAX;
        }
    }

}