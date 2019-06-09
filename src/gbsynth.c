/*
** File: src/gbsynth.c
**
** Author: stoneface86
*/

#include "gbsynth.h"

#include <assert.h>
#include <stdlib.h>

#define NULLCHECK(var) assert(var != NULL)



struct GbsSynth {
    float samplingRate;
    GbsChType channel;
    GbsChRegUnion reg;
    float currentSample;
};

GbsErr gbs_init(GbsSynth **synthVar, GbsChType channel, float samplingRate) {
    NULLCHECK(synthVar);
    GbsSynth *synth = (GbsSynth*)malloc(sizeof(struct GbsSynth));
    if (synth == NULL) {
        return GBS_E_MEM;
    }
    synth->channel = channel;
    synth->samplingRate = samplingRate;


    *synthVar = synth;
    return GBS_E_NONE;
}

GbsErr gbs_deinit(GbsSynth *synth) {
    NULLCHECK(synth);
    free(synth);
    return GBS_E_NONE;
}

float gbs_currentSample(GbsSynth *synth, GbsChType chan) {
    NULLCHECK(synth);
    return synth->currentSample;
}

GbsErr gbs_getRegisters(GbsSynth *synth, GbsChType chan, GbsChRegUnion *reg) {
    NULLCHECK(synth);
    NULLCHECK(reg);

    *reg = synth->reg;

    return GBS_E_NONE;
}

// channel 1

GbsErr gbs_ch1_setSweepTime(GbsSynth *synth, uint8_t ts) {
    NULLCHECK(synth);

    if (synth->channel != GBS_CH1) {
        return GBS_E_WRONG_CHANNEL;
    }
    
    if (ts > GBS_MAX_SWEEP_TIME) {
        return GBS_E_PARAMETER;
    }
    uint8_t nr10 = synth->reg.ch1.nr10;
    nr10 &= 0x1F; // reset the current time value
    nr10 |= ts << 5; // set the new value
    synth->reg.ch1.nr10 = nr10;

    // TODO: update the oscillator

    return GBS_E_NONE;
}

GbsErr gbs_ch1_setSweepMode(GbsSynth *synth, GbsSweepMode mode) {
    NULLCHECK(synth);

    return GBS_E_PARAMETER;
}

GbsErr gbs_ch1_setSweepShift(GbsSynth *synth, uint8_t n) {
    NULLCHECK(synth);

    return GBS_E_PARAMETER;
}

//
// Channel 3 settings
//
GbsErr gbs_ch3_setEnable(GbsSynth *synth, bool enabled) {
    NULLCHECK(synth);

    return GBS_E_PARAMETER;
}

GbsErr gbs_ch3_setWaveform(GbsSynth *synth, uint8_t waveform[16]) {
    NULLCHECK(synth);

    return GBS_E_PARAMETER;
}

GbsErr gbs_ch3_setWaveformLevel(GbsSynth *synth, GbsWaveformLevel level) {
    NULLCHECK(synth);

    return GBS_E_PARAMETER;
}

//
// Channel 4 settings
//
GbsErr gbs_ch4_setScf(GbsSynth *synth, uint8_t scf) {
    NULLCHECK(synth);

    return GBS_E_PARAMETER;
}

GbsErr gbs_ch4_setStep(GbsSynth *synth, GbsStepCount steps) {
    NULLCHECK(synth);

    return GBS_E_PARAMETER;
}

GbsErr gbs_ch4_setDrf(GbsSynth *synth, uint8_t drf) {
    NULLCHECK(synth);

    return GBS_E_PARAMETER;
}


//
// general settings
//
GbsErr gbs_setDuty(GbsSynth *synth, GbsDuty duty) {
    NULLCHECK(synth);

    return GBS_E_PARAMETER;
}

GbsErr gbs_setLength(GbsSynth *synth, uint8_t length) {
    NULLCHECK(synth);

    return GBS_E_PARAMETER;
}

GbsErr gbs_setEnvSteps(GbsSynth *synth, uint8_t steps) {
    NULLCHECK(synth);

    return GBS_E_PARAMETER;
}

GbsErr gbs_setEnvMode(GbsSynth *synth, GbsEnvMode mode) {
    NULLCHECK(synth);

    return GBS_E_PARAMETER;
}

GbsErr gbs_setEnvLength(GbsSynth *synth, uint8_t length) {
    NULLCHECK(synth);

    return GBS_E_PARAMETER;
}

GbsErr gbs_setFreq(GbsSynth *synth, uint16_t freq) {
    NULLCHECK(synth);

    return GBS_E_PARAMETER;
}
