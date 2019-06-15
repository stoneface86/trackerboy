/*
** File: src/envelope.c
**
** Author: stoneface86
**
** Envelope function (channels 1, 2 and 4)
*/

#include "envelope.h"

#include <stdbool.h>
#include <math.h>

//
// The time per step in the envelope is given by the formula, n / 64 seconds
// where n is a 3-bit value. If n is 0, then there is no envelope function
//
#define timePerStep(n) (n / 64.0f)

//
// Value of the envelope is the step, n, divided by the maximum number of steps.
//
#define envValue(n) ((float)(n) / (float)GBS_MAX_ENV_STEPS)

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


float envelope_apply(Envelope *env, float sample) {
    if (fabsf(sample) > env->value) {
        // clamp the sample to the envelope
        return env->value * copysignf(1.0f, sample);
    } else {
        return sample;
    }
}

void envelope_init(Envelope *env, float samplingRate, uint8_t steps, GbsEnvMode mode, uint8_t length) {
    env->steps = steps;
    env->mode = mode;
    env->length = length;

    env->samplesPerStep = (unsigned)(samplingRate * ENV_TIMING_TABLE[length]);
    env->stepCounter = steps;
    env->counter = 0;

    env->value = ENV_VALUE_TABLE[steps];

}

void envelope_setStep(Envelope *env, uint8_t step) {
    env->steps = step;

    env->stepCounter = step;
    env->value = ENV_VALUE_TABLE[step];
}

void envelope_setMode(Envelope *env, GbsEnvMode mode) {
    env->mode = mode;
}

void envelope_setLength(Envelope *env, float samplingRate, uint8_t length) {
    env->length = length;
    env->samplesPerStep = (unsigned)(samplingRate * ENV_TIMING_TABLE[length]);
}


void envelope_tick(Envelope *env) {
    // no envelope function if the length was 0
    if (env->samplesPerStep != 0) {
        bool newstep = false;
        if (env->counter++ >= env->samplesPerStep) {
            if (env->mode == GBS_ENV_AMPLIFY) {
                if (env->stepCounter < GBS_MAX_ENV_STEPS) {
                    ++env->stepCounter;
                    newstep = true;
                }
            } else {
                if (env->stepCounter > 0) {
                    --env->stepCounter;
                    newstep = true;
                }
            }

            if (newstep) {
                env->value = ENV_VALUE_TABLE[env->stepCounter];
            }
            env->counter = 0;
        }
    }
}