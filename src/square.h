/*
** File: src/square.h
**
** Author: stoneface86
**
** Contains functions for synthesizing a square wave
*/

#pragma once

#include <stdint.h>

typedef struct SquareOsc {
    float samplingRate;
    float frequency;
    float duty;
    unsigned samplesPerDuty;    // number of samples for the "on" part of the wave
    unsigned samplesPerPeriod;  // number of samples per period
    unsigned counter;
} SquareOsc;

void square_init(SquareOsc *osc, float samplingRate, float frequency, float duty);

void square_setFrequency(SquareOsc *osc, float frequency);

void square_setDuty(SquareOsc *osc, float duty);

float square_nextSample(SquareOsc *osc);
