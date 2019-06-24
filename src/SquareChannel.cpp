
#include "gbsynth.h"

#include <cmath>

#define calcSamplesPerPeriod(f) ((unsigned)roundf(samplingRate / f))
#define calcSamplesPerDuty(duty) ((unsigned)roundf(samplesPerPeriod * DUTY_TABLE[duty]))
#define calcSamplesPerSweep(ts) ((unsigned)roundf(samplingRate * SWEEP_TIMING[ts]))
#define calcSweepTime(ts) (ts / 128.0f)

namespace gbsynth {

    static const float DUTY_TABLE[] = {
        0.125f,
        0.25f,
        0.5f,
        0.75f
    };

    static const float SWEEP_TIMING[] = {
        0.0f,
        calcSweepTime(1),
        calcSweepTime(2),
        calcSweepTime(3),
        calcSweepTime(4),
        calcSweepTime(5),
        calcSweepTime(6),
        calcSweepTime(7),
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
        samplesPerSweep = calcSamplesPerSweep(sweepTime);
        sweepCounter = 0;
        periodCounter = 0;
    }

    size_t SquareChannel::generate(float buf[], size_t nsamples) {
        int32_t sweepfreq;
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

            if (sweepEnabled && sweepTime != 0) {
                if (++sweepCounter >= samplesPerSweep) {
                    sweepCounter = 0;
                    if (sweepShift != 0) {
                        sweepfreq = frequency >> sweepShift;
                        if (sweepMode == SWEEP_SUBTRACTION) {
                            sweepfreq = frequency - sweepfreq;
                            if (sweepfreq < 0) {
                                sweepfreq = frequency; // no change
                            }
                        } else {
                            sweepfreq = frequency + sweepfreq;
                            if (sweepfreq > MAX_FREQUENCY) {
                                // sweep will overflow, stop the sound
                                enabled = false;
                                return i + 1;
                            }
                        }

                        
                        setFrequency((uint16_t)sweepfreq);
                    }
                }
            }
        }
        // apply the envelope
        apply(buf, nsamples);
        return nsamples;
    }

    void SquareChannel::getRegisters(ChRegUnion* reg) {
        
    }

    void SquareChannel::reset() {
        EnvChannel::reset();
        periodCounter = 0;
        sweepCounter = 0;
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

    void SquareChannel::setSweepTime(uint8_t ts) {
        if (ts > MAX_SWEEP_TIME) {
            ts = MAX_SWEEP_TIME;
        }
        sweepTime = ts;
        samplesPerSweep = calcSamplesPerSweep(ts);
    }

    void SquareChannel::setSweepMode(SweepMode mode) {
        sweepMode = mode;
    }

    void SquareChannel::setSweepShift(uint8_t shift) {
        if (shift > MAX_SWEEP_SHIFT) {
            shift = MAX_SWEEP_SHIFT;
        }
        sweepShift = shift;
    }

}