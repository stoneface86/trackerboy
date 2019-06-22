
#include "gbsynth2.h"

#include <cmath>

//
// The time per step in the envelope is given by the formula, n / 64 seconds
// where n is a 3-bit value. If n is 0, then there is no envelope function
//
#define timePerStep(n) (n / 64.0f)

//
// Value of the envelope is the step, n, divided by the maximum number of steps.
//
#define envValue(n) ((float)(n) / (float)MAX_ENV_STEPS)

#define calcSamples(length) ((unsigned)roundf(samplingRate * ENV_TIMING_TABLE[length]))

namespace gbsynth {

    static float ENV_VALUE_TABLE[] = {
        envValue(0),
        envValue(1),
        envValue(2),
        envValue(3),
        envValue(4),
        envValue(5),
        envValue(6),
        envValue(7),
        envValue(8),
        envValue(9),
        envValue(10),
        envValue(11),
        envValue(12),
        envValue(13),
        envValue(14),
        envValue(15)
    };

    static float ENV_TIMING_TABLE[] = {
        timePerStep(0),
        timePerStep(1),
        timePerStep(2),
        timePerStep(3),
        timePerStep(4),
        timePerStep(5),
        timePerStep(6),
        timePerStep(7)
    };

    EnvChannel::EnvChannel(float samplingRate) : Channel(samplingRate) {
        envLength = DEFAULT_ENV_LENGTH;
        envMode = (EnvMode)DEFAULT_ENV_MODE;
        envSteps = DEFAULT_ENV_STEPS;
        samplesPerStep = calcSamples(envLength);
        stepCounter = envSteps;
        envCounter = 0;
        envelope = ENV_VALUE_TABLE[envSteps];

    }

    void EnvChannel::apply(float buf[], size_t bufsize) {
        if (envLength != 0) {
            // envelope is not applied when envLength == 0
            for (size_t i = 0; i != bufsize; ++i) {
                buf[i] = buf[i] * envelope;
                if (envCounter++ >= samplesPerStep) {
                    if (envMode == ENV_AMPLIFY) {
                        if (stepCounter < MAX_ENV_STEPS) {
                            ++stepCounter;
                            envelope = ENV_VALUE_TABLE[stepCounter];
                        }
                    } else {
                        if (stepCounter > 0) {
                            --stepCounter;
                            envelope = ENV_VALUE_TABLE[stepCounter];
                        }
                    }
                    envCounter = 0;
                }
            }

        }
    }

    void EnvChannel::setEnvLength(uint8_t length) {
        if (length > MAX_ENV_LENGTH) {
            length = MAX_ENV_LENGTH;
        }
        envLength = length;
        samplesPerStep = calcSamples(length);
    }

    void EnvChannel::setEnvMode(EnvMode mode) {
        this->envMode = mode;
    }

    void EnvChannel::setEnvStep(uint8_t step) {
        if (step > MAX_ENV_STEPS) {
            step = MAX_ENV_STEPS;
        }
        envSteps = step;
        stepCounter = step;
        envelope = ENV_VALUE_TABLE[step];
    }
    
    uint8_t EnvChannel::encodeEnvRegister() {
        uint8_t reg;
        reg = envSteps << 4;
        reg |= envMode << 3;
        reg |= envLength;
        return reg;
    }

    void EnvChannel::reset() {
        stepCounter = envSteps;
        envCounter = 0;
        envelope = ENV_VALUE_TABLE[envSteps];
    }

}