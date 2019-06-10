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

#define timePerStep(n) (n / 64.0f)
#define envValue(n) ((float)(n) / (float)GBS_MAX_ENV_STEPS)


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

    env->samplesPerStep = (unsigned)(samplingRate * timePerStep(length));
    env->stepCounter = steps;
    env->counter = 0;

    env->value = envValue(steps);

}

void envelope_setStep(Envelope *env, uint8_t step) {
    env->steps = step;

    env->stepCounter = step;
    env->value = envValue(step);
}

void envelope_setMode(Envelope *env, GbsEnvMode mode) {
    env->mode = mode;
}

void envelope_setLength(Envelope *env, float samplingRate, uint8_t length) {
    env->length = length;
    env->samplesPerStep = (unsigned)(samplingRate * timePerStep(length));
}


void envelope_tick(Envelope *env) {
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
            env->value = envValue(env->stepCounter);
        }
        env->counter = 0;
    }
}