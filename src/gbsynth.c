/*
** File: src/gbsynth.c
**
** Author: stoneface86
*/

#include "gbsynth.h"

#include <assert.h>
#include <stdlib.h>

#define NULLCHECK(var) assert(var != NULL)

typedef struct SquareOsc {
    unsigned ticksPerDuty;
    unsigned ticksPerPeriod;
    unsigned ticksPerEnvStep;
    unsigned envSteps;
    unsigned ticks;
} SquareOsc;

struct GbsSynth {
    float samplingRate;
    GbsCh1Reg ch1;
    GbsCh2Reg ch2;
    GbsCh3Reg ch3;
    GbsCh4Reg ch4;
    float currentSamples[4];


};

GbsErr gbs_init(GbsSynth **synthVar, float samplingRate) {
    NULLCHECK(synthVar);
    GbsSynth *synth = (GbsSynth*)malloc(sizeof(struct GbsSynth));
    if (synth == NULL) {
        return GBS_E_MEM;
    }
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
    return synth->currentSamples[chan];
}

GbsErr gbs_getRegisters(GbsSynth *synth, GbsChType chan, GbsChRegUnion *reg) {
    NULLCHECK(synth);
    NULLCHECK(reg);

    switch (chan) {
        case GBS_CH1:
            reg->ch1 = synth->ch1;
            break;
        case GBS_CH2:
            reg->ch2 = synth->ch2;
            break;
        case GBS_CH3:
            reg->ch3 = synth->ch3;
            break;
        case GBS_CH4:
            reg->ch4 = synth->ch4;
            break;
        default:
            return GBS_E_PARAMETER;
    }

    return GBS_E_NONE;
}

// channel 1

GbsErr gbs_ch1_setSweepTime(GbsSynth *synth, uint8_t ts) {
    NULLCHECK(synth);
    
    if (ts > GBS_SWEEP_TIME_MAX) {
        return GBS_E_PARAMETER;
    }
    uint8_t nr10 = synth->ch1.nr10;
    nr10 &= 0x1F; // reset the current time value
    nr10 |= ts << 5; // set the new value
    synth->ch1.nr10 = nr10;

    // TODO: update the oscillator

    return GBS_E_NONE;
}

GbsErr gbs_ch1_setSweepType(GbsSynth *synth, uint8_t addition);

GbsErr gbs_ch1_setSweepShift(GbsSynth *synth, uint8_t n);

//void gbs_decodeCh2(GbsCh2 *ch2, GbsCh2Reg *reg) {
//	ch2->duty = (GbsDuty)(reg->nr21 >> 6);
//	ch2->length = reg->nr21 & 0x3f;
//	ch2->envSteps = reg->nr22 >> 4;
//	ch2->envDirection = (GbsEnvMode)((reg->nr22 >> 3) & 0x1);
//	ch2->envLength = reg->nr22 & 0x7;
//	ch2->frequency = ((reg->nr24 & 0x7) << 8) | reg->nr23;
//}
//
//void gbs_encodeCh2(GbsCh2 *ch2, GbsCh2Reg *reg) {
//    // NR21
//    // bits 0-5: sound length
//    // bits 6-7: duty
//    reg->nr21 = ((ch2->duty & 3) << 6) |
//                (ch2->length & 0x3f);
//    // NR22
//    // bits 0-2: envelope step length
//    // bit    3: envelope direction
//    // bits 4-7: enevelop steps
//    reg->nr22 = ((ch2->envSteps & 0xF) << 4) |
//                (ch2->envDirection << 3) |
//                (ch2->envLength & 0x7);
//
//    // NR23
//    // bits 0-7: lower 8 bits of the frequency data
//    reg->nr23 = ch2->frequency & 0xFF;
//
//    // NR24
//    // bits 0-3: higher 3 bits of the frequency data
//    reg->nr24 = (ch2->frequency >> 8) & 0x7;
//}
//
//void gbs_setEnvSteps(GbsSynth *synth, GbsChType chan, uint8_t steps) {
//	switch (chan) {
//		case GBS_CH_SOUND1:
//			break;
//		case GBS_CH_SOUND2:
//			break;
//		default:
//			return -1;
//	}
//}