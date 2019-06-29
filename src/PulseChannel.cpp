
#include "gbsynth.h"

#include <cmath>
#include <algorithm>

using std::min;

#define calcSamplesPerSweep(ts) ((unsigned)roundf(samplingRate * SWEEP_TIMING[ts]))
#define calcSweepTime(ts) (ts / 128.0f)

#define WAVE_SAMPLES 8

namespace gbsynth {

    static uint8_t WAVE_TABLE[][WAVE_SAMPLES] = {
        // Duty 12.5%:  00000001
        {SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MAX},
        // Duty 25%:    10000001
        {SAMPLE_MAX, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MAX},
        // Duty 50%:    10000111
        {SAMPLE_MAX, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MAX},
        // Duty 75%:    01111110
        {SAMPLE_MIN, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MIN}
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

    PulseChannel::PulseChannel(float samplingRate, bool enableSweep) : 
        EnvChannel(samplingRate), 
        osc(WAVE_TABLE[DEFAULT_DUTY], WAVE_SAMPLES) 
    {
        sweepEnabled = enableSweep;
        frequency = DEFAULT_FREQUENCY;
        duty = (Duty)DEFAULT_DUTY;
        sweepMode = (SweepMode)DEFAULT_SWEEP_MODE;
        sweepTime = DEFAULT_SWEEP_TIME;
        sweepShift = DEFAULT_SWEEP_SHIFT;

        osc.setFrequency(samplingRate, fromGbFreq(frequency));
        samplesPerSweep = calcSamplesPerSweep(sweepTime);
        sweepCounter = 0;
    }

    size_t PulseChannel::generate(uint8_t buf[], size_t nsamples) {
        //int32_t sweepfreq;
        //uint8_t sample;

        if (sweepEnabled) {

            size_t nfill = samplesPerSweep - sweepCounter;
            uint8_t *bp = buf;
            size_t count = 0;
            int16_t sweepfreq;

            while (count != nsamples) {
                nfill = min(nfill, nsamples - count);
                bp = osc.fill(bp, nfill);
                count += nfill;
                sweepCounter += nfill;
                if (sweepCounter >= samplesPerSweep) {
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
                                return count;
                            }
                        }

                        setFrequency((uint16_t)sweepfreq);
                    }
                }
                nfill = samplesPerSweep;
                
            }
        } else {
            osc.fill(buf, nsamples);
        }

        // apply the envelope
        apply(buf, nsamples);
        return nsamples;
    }

    void PulseChannel::getRegisters(ChRegUnion* reg) {
        
    }

    void PulseChannel::reset() {
        EnvChannel::reset();
        osc.reset();
        sweepCounter = 0;
    }

    void PulseChannel::setDuty(Duty duty) {
        this->duty = duty;
        osc.setWaveform(WAVE_TABLE[duty], WAVE_SAMPLES);
    }

    void PulseChannel::setFrequency(uint16_t frequency) {
        this->frequency = frequency;
        float f = fromGbFreq(frequency);
        osc.setFrequency(samplingRate, f);
    }

    void PulseChannel::setFrequency(float frequency) {
        this->frequency = toGbFreq(frequency);
        osc.setFrequency(samplingRate, frequency);
    }

    void PulseChannel::setSweepTime(uint8_t ts) {
        if (ts > MAX_SWEEP_TIME) {
            ts = MAX_SWEEP_TIME;
        }
        sweepTime = ts;
        samplesPerSweep = calcSamplesPerSweep(ts);
    }

    void PulseChannel::setSweepMode(SweepMode mode) {
        sweepMode = mode;
    }

    void PulseChannel::setSweepShift(uint8_t shift) {
        if (shift > MAX_SWEEP_SHIFT) {
            shift = MAX_SWEEP_SHIFT;
        }
        sweepShift = shift;
    }

}