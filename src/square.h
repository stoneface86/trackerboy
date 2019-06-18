/*
** File: src/square.h
**
** Author: stoneface86
**
** Contains functions for synthesizing a square wave
*/

#pragma once

#include "gbsynth.h"
#include <stdint.h>

typedef struct SquareOsc {
    uint16_t frequency;
    GbsDuty duty;
    unsigned samplesPerDuty;    // number of samples for the "on" part of the wave
    unsigned samplesPerPeriod;  // number of samples per period
    unsigned counter;           // period counter, or position in the waveBuf
} SquareOsc;

void square_init(SquareOsc *osc);

void square_setFrequency(SquareOsc *osc, float samplingRate, uint16_t frequency);

void square_setDuty(SquareOsc *osc, GbsDuty duty);

float square_nextSample(SquareOsc *osc);

void square_fill(SquareOsc* osc, float buf[], size_t nsamples);
