/*
** File: src/gbsynth.c
**
** Author: stoneface86
*/

#include "gbsynth.h"

#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include "square.h"
#include "envelope.h"

#define NULLCHECK(var) assert(var != NULL)


typedef union OscUnion {
    SquareOsc square;
} OscUnion;


struct GbsSynth {
    float samplingRate;
    GbsChType channel;
    float currentSample;
    OscUnion osc;
    Envelope *env;
};

GbsErr gbs_init(GbsSynth **synthVar, GbsChType channel, float samplingRate) {
    NULLCHECK(synthVar);
    GbsSynth *synth = (GbsSynth*)malloc(sizeof(struct GbsSynth));
    if (synth == NULL) {
        return GBS_E_MEM;
    }
    synth->channel = channel;
    synth->samplingRate = samplingRate;

    switch (channel) {
        case GBS_CH1:
            // TODO: initialize sweep
            // fall-through
        case GBS_CH2:
            square_init(&synth->osc.square);
            square_setDuty(&synth->osc.square, GBS_DEFAULT_DUTY);
            square_setFrequency(&synth->osc.square, samplingRate, GBS_DEFAULT_FREQUENCY);
            break;
        case GBS_CH3:
            break;
        case GBS_CH4:
            break;
    }

    // all channels have an envelope except for channel 3
    if (channel == GBS_CH3) {
        synth->env = NULL;
    } else {
        Envelope *env = (Envelope*)malloc(sizeof(Envelope));
        if (env == NULL) {
            free(synth);
            return GBS_E_MEM;
        }
        envelope_init(env, samplingRate, GBS_DEFAULT_ENV_STEPS, GBS_DEFAULT_ENV_MODE, GBS_DEFAULT_ENV_LENGTH);
    }


    *synthVar = synth;
    return GBS_E_NONE;
}

GbsErr gbs_deinit(GbsSynth *synth) {
    NULLCHECK(synth);
    if (synth->env != NULL) {
        free(synth->env);
    }
    free(synth);
    return GBS_E_NONE;
}

float gbs_currentSample(GbsSynth *synth) {
    NULLCHECK(synth);
    return synth->currentSample;
}

GbsErr gbs_getRegisters(GbsSynth *synth, GbsChRegUnion *reg) {
    NULLCHECK(synth);
    NULLCHECK(reg);

    

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
    //uint8_t nr10 = synth->reg.ch1.nr10;
    //nr10 &= 0x1F; // reset the current time value
    //nr10 |= ts << 5; // set the new value
    //synth->reg.ch1.nr10 = nr10;

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

    switch (synth->channel) {
        case GBS_CH1:
        case GBS_CH2:
            square_setDuty(&synth->osc.square, duty);
            return GBS_E_NONE;
        default:
            return GBS_E_WRONG_CHANNEL;
    }
}

GbsErr gbs_setLength(GbsSynth *synth, uint8_t length) {
    NULLCHECK(synth);

    return GBS_E_PARAMETER;
}

GbsErr gbs_setEnvSteps(GbsSynth *synth, uint8_t steps) {
    NULLCHECK(synth);
    if (steps > GBS_MAX_ENV_STEPS) {
        return GBS_E_PARAMETER;
    }

    if (synth->env != NULL) {
        envelope_setStep(synth->env, steps);
        return GBS_E_NONE;
    } else {
        return GBS_E_WRONG_CHANNEL;
    }
}

GbsErr gbs_setEnvMode(GbsSynth *synth, GbsEnvMode mode) {
    NULLCHECK(synth);
    if (synth->env != NULL) {
        envelope_setMode(synth->env, mode);
        return GBS_E_NONE;
    } else {
        return GBS_E_WRONG_CHANNEL;
    }
}

GbsErr gbs_setEnvLength(GbsSynth *synth, uint8_t length) {
    NULLCHECK(synth);
    if (length > GBS_MAX_ENV_LENGTH) {
        return GBS_E_PARAMETER;
    }

    if (synth->env != NULL) {
        envelope_setLength(synth->env, synth->samplingRate, length);
        return GBS_E_NONE;
    } else {
        return GBS_E_WRONG_CHANNEL;
    }
}

GbsErr gbs_setFreq(GbsSynth *synth, uint16_t freq) {
    NULLCHECK(synth);

    if (freq > GBS_MAX_FREQUENCY) {
        return GBS_E_PARAMETER;
    }

    switch (synth->channel) {
        case GBS_CH1:
        case GBS_CH2:
            square_setFrequency(&synth->osc.square, synth->samplingRate, freq);
            return GBS_E_NONE;
        case GBS_CH3:
            return GBS_E_NONE;
        default:
            return GBS_E_WRONG_CHANNEL;
    }
}

//
// utility functions
//

float gbs_freq(uint16_t freq) {
    if (freq > GBS_MAX_FREQUENCY) {
        freq = GBS_MAX_FREQUENCY; // clamp
    }
    return 131072.0f / (2048 - freq);
}

uint16_t gbs_arcfreq(float freq) {

    float calc = 2048.0f - (131072.0f / freq);
    if (isnormal(calc) && calc > 0.0f) {
        calc = roundf(calc);
        if (calc < 2048.0f) {
            return (uint16_t)calc;   
        }
    }


    return GBS_MAX_FREQUENCY;
}