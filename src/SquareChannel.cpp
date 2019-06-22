
#include "gbsynth2.h"

#include <cmath>

#define calcSamplesPerPeriod(f) ((unsigned)roundf(samplingRate / f))
#define calcSamplesPerDuty(duty) ((unsigned)roundf(samplesPerPeriod * DUTY_TABLE[duty]))

namespace gbsynth {

    static const float DUTY_TABLE[] = {
        0.125f,
        0.25f,
        0.5f,
        0.75f
    };

    SquareChannel::SquareChannel(float samplingRate, bool enableSweep) : EnvChannel(samplingRate) {
        sweepEnabled = enableSweep;
        frequency = DEFAULT_FREQUENCY;
        duty = (Duty)DEFAULT_DUTY;
        sweepMode = (SweepMode)DEFAULT_SWEEP_MODE;
        sweepTime = DEFAULT_SWEEP_TIME;
        sweepShift = DEFAULT_SWEEP_SHIFT;

        float f = fromGbFreq(frequency);
        samplesPerPeriod = calcSamplesPerPeriod(f);
        samplesPerDuty = calcSamplesPerDuty(duty);
        periodCounter = 0;
    }

    void SquareChannel::generate(float buf[], size_t nsamples) {
        float sample;
        for (size_t i = 0; i != nsamples; ++i) {
            if (periodCounter < samplesPerDuty) {
                sample = 1.0f;
            } else {
                sample = -1.0f;
            }
            buf[i] = sample;

            //update counter
            if (++periodCounter >= samplesPerPeriod) {
                periodCounter = 0;
            }
        }
        // apply the envelope
        apply(buf, nsamples);
    }

    void SquareChannel::getRegisters(ChRegUnion* reg) {
        
    }

    void SquareChannel::reset() {
        EnvChannel::reset();
        periodCounter = 0;
    }

    void SquareChannel::setDuty(Duty duty) {
        this->duty = duty;
        samplesPerDuty = calcSamplesPerDuty(duty);
    }

    void SquareChannel::setFrequency(uint16_t frequency) {
        this->frequency = frequency;
        float f = fromGbFreq(frequency);
        samplesPerPeriod = calcSamplesPerPeriod(f);
        samplesPerDuty = calcSamplesPerDuty(duty);
    }

    void SquareChannel::setFrequency(float frequency) {
        this->frequency = toGbFreq(frequency);
        samplesPerPeriod = calcSamplesPerPeriod(frequency);
        samplesPerDuty = calcSamplesPerDuty(duty);
    }

}