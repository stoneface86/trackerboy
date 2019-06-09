/*
** File: src/square.c
**
** Author: stoneface86
**
** Contains functions for synthesizing a square wave
*/

#include "square.h"

void square_init(SquareOsc *osc, float samplingRate, float frequency, float duty) {
    osc->samplingRate = samplingRate;
    osc->duty = duty;
    square_setFrequency(osc, frequency);
    osc->counter = 0;
}
void square_setFrequency(SquareOsc *osc, float frequency) {
    osc->frequency = frequency;
    unsigned samplesPerPeriod = (unsigned)(osc->samplingRate / frequency);
    osc->samplesPerPeriod = samplesPerPeriod;
    osc->samplesPerDuty = (unsigned)(samplesPerPeriod * osc->duty);
}

void square_setDuty(SquareOsc *osc, float duty) {
    osc->duty = duty;
    osc->samplesPerDuty = (unsigned)(osc->samplesPerPeriod * duty);
}

float square_nextSample(SquareOsc *osc) {
    float sample;

    if (osc->counter < osc->samplesPerDuty) {
        sample = 1.0f;
    } else {
        sample = -1.0f;
    }

    // update counter
    if (++osc->counter >= osc->samplesPerPeriod) {
        osc->counter = 0;
    }
    

    return sample;
}