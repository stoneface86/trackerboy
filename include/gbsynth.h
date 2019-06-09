/*
** File: include/gbsynth.h
**
** Author: stoneface86
**
*/

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum GbsErr {
    GBS_E_NONE = 0,         // no error
    GBS_E_MEM,              // out of memory
    GBS_E_WRONG_CHANNEL,    // setting cannot be applied to given channel
    GBS_E_PARAMETER         // parameter out of range
} GbsErr;

typedef enum GbsChType {
	GBS_CH1,  // square-wave with sweep, envelope
	GBS_CH2,  // square-wave with envelope
	GBS_CH3,  // wave playback
	GBS_CH4   // white noise generator
} GbsChType;

typedef enum GbsDuty {
    GBS_DUTY_125 = 0,	// 12.5%
    GBS_DUTY_25 = 1,	// 25%
    GBS_DUTY_50 = 2,	// 50%
    GBS_DUTY_75 = 3		// 75%
} GbsDuty;

typedef enum GbsEnvMode {
    GBS_ENV_ATTENUATE = 0,
    GBS_ENV_AMPLIFY = 1,
} GbsEnvMode;

typedef enum GbsSweepMode {
    GBS_SWEEP_ADDITION = 0,
    GBS_SWEEP_SUBTRACTION = 1
} GbsSweepMode;

typedef enum GbsWaveOutput {
	GBS_WAVE_MUTE    = 0,
	GBS_WAVE_WHOLE   = 1,
	GBS_WAVE_HALF    = 2,
	GBS_WAVE_QUARTER = 3
} GbsWaveOutput;

typedef struct GbsCh1Reg {
    uint8_t nr10;
    uint8_t nr11;
    uint8_t nr12;
    uint8_t nr13;
    uint8_t nr14;
} GbsCh1Reg;

typedef struct GbsCh2Reg {
    uint8_t nr21;
    uint8_t nr22;
    uint8_t nr23;
    uint8_t nr24;
} GbsCh2Reg;

typedef struct GbsCh3Reg {
    uint8_t nr30;
    uint8_t nr31;
    uint8_t nr32;
    uint8_t nr33;
    uint8_t nr34;
} GbsCh3Reg;

typedef struct GbsCh3Reg {
    uint8_t nr41;
    uint8_t nr42;
    uint8_t nr43;
    uint8_t nr44;
} GbsCh4Reg;

typedef union GbsChRegUnion {
    GbsCh1Reg ch1;
    GbsCh2Reg ch2;
    GbsCh3Reg ch3;
    GbsCh4Reg ch4;
} GbsChRegUnion;

typedef struct GbsSynth GbsSynth;

GbsErr gbs_getRegisters(GbsSynth *synth, GbsChType chan, GbsChRegUnion *reg);

// ========================================================
// Synthesizer settings
// ========================================================

//
// Channel 1 settings
//
GbsErr gbs_ch1_setSweepTime(GbsSynth *synth, uint8_t ts);

GbsErr gbs_ch1_setSweepType(GbsSynth *synth, uint8_t addition);

GbsErr gbs_ch1_setSweepShift(GbsSynth *synth, uint8_t n);

//
// Channel 3 settings
//
GbsErr gbs_ch3_setEnable(GbsSynth *synth, bool enabled);

GbsErr gbs_ch3_setWaveform(GbsSynth *synth, uint8_t waveform[16]);


//
// general settings
//
GbsErr gbs_setDuty(GbsSynth *synth, GbsChType chan, GbsDuty duty);

GbsErr gbs_setLength(GbsSynth *synth, GbsChType chan, uint8_t length);

GbsErr gbs_setEnvSteps(GbsSynth *synth, GbsChType chan, uint8_t steps);

GbsErr gbs_setEnvMode(GbsSynth *synth, GbsChType chan, GbsEnvMode mode);

GbsErr gbs_setEnvLength(GbsSynth *synth, GbsChType chan, uint8_t length);

GbsErr gbs_setFreq(GbsSynth *synth, GbsChType chan, uint16_t freq);


//
// synth
//

GbsErr gbs_init(GbsSynth **synth, float samplingRate);

GbsErr gbs_deinit(GbsSynth *synth);

float gbs_currentSample(GbsSynth *synth, GbsChType chan);

GbsErr gbs_tick(GbsSynth *synth);