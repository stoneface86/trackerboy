/*
** File: src/envelope.h
**
** Author: stoneface86
**
** Envelope function (channels 1, 2 and 4)
*/

#include "gbsynth.h"

typedef struct Envelope {
    uint8_t steps;
    GbsEnvMode mode;
    uint8_t length;
    unsigned samplesPerStep; // samples per envelope step
    unsigned stepCounter;    // current step
    unsigned counter;        // sample counter

    float value; // current value of the envelope
} Envelope;

float envelope_apply(Envelope *env, float sample);

void envelope_applyBuf(Envelope* env, float buf[], size_t nsamples);

void envelope_init(Envelope *env, float samplingRate, uint8_t steps, GbsEnvMode mode, uint8_t length);

void envelope_setStep(Envelope *env, uint8_t step);

void envelope_setMode(Envelope *env, GbsEnvMode mode);

void envelope_setLength(Envelope *env, float samplingRate, uint8_t length);

void envelope_tick(Envelope *env);