/*
** File: src/square.c
**
** Author: stoneface86
**
** Contains functions for synthesizing a square wave
*/

#include "square.h"

void square_init(SquareOsc *osc) {
    osc->samplesPerDuty = 0;
    osc->samplesPerPeriod = 0;
    osc->counter = 0;
}
void square_setFrequency(SquareOsc *osc, float samplingRate, uint16_t frequency) {
    osc->frequency = frequency;
    unsigned samplesPerPeriod = (unsigned)(samplingRate / gbs_freq(frequency));
    osc->samplesPerPeriod = samplesPerPeriod;
    osc->samplesPerDuty = (unsigned)(samplesPerPeriod * osc->duty);
}

void square_setDuty(SquareOsc *osc, float duty) {
    osc->duty = duty;
    osc->samplesPerDuty = (unsigned)(osc->samplesPerPeriod * duty);
}

float square_nextSample(SquareOsc *osc) {
    float sample = 0.0f;

    // output nothing if no frequency was set
    if (osc->samplesPerPeriod == 0) {

        if (osc->counter < osc->samplesPerDuty) {
            sample = 1.0f;
        } else {
            sample = -1.0f;
        }

        // update counter
        if (++osc->counter >= osc->samplesPerPeriod) {
            osc->counter = 0;
        }
    }
    

    return sample;
}