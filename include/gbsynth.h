/*
** File: include/gbsynth.h
**
** Author: stoneface86
**
*/

#pragma once

#include <stdbool.h>
#include <stdint.h>

// maximum values for parameters
#define GBS_MAX_SWEEP_TIME      0x7
#define GBS_MAX_SWEEP_SHIFT     0x7
#define GBS_MAX_LENGTH          0x3F
#define GBS_MAX_ENV_STEPS       0xF
#define GBS_MAX_ENV_LENGTH      0x7
#define GBS_MAX_FREQUENCY       0x7FF
#define GBS_MAX_WAVE_LENGTH     0xFF

// defaults
#define GBS_DEFAULT_FREQUENCY   0
#define GBS_DEFAULT_DUTY        GBS_DUTY_75
#define GBS_DEFAULT_ENV_STEPS   0
#define GBS_DEFAULT_ENV_LENGTH  0
#define GBS_DEFAULT_ENV_MODE    GBS_ENV_ATTENUATE
#define GBS_DEFAULT_SWEEP_TIME  GBS_MAX_SWEEP_TIME
#define GBS_DEFAULT_SWEEP_MODE  GBS_SWEEP_ADDITION
#define GBS_DEFAULT_SWEEP_SHIFT 0


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
    GBS_DUTY_125 = 0,   // 12.5%
    GBS_DUTY_25 = 1,    // 25%
    GBS_DUTY_50 = 2,    // 50%
    GBS_DUTY_75 = 3     // 75%
} GbsDuty;

typedef enum GbsEnvMode {
    GBS_ENV_ATTENUATE = 0,
    GBS_ENV_AMPLIFY = 1,
} GbsEnvMode;

typedef enum GbsSweepMode {
    GBS_SWEEP_ADDITION = 0,
    GBS_SWEEP_SUBTRACTION = 1
} GbsSweepMode;

typedef enum GbsStepCount {
    GBS_STEPS_15 = 0,
    GBS_STEPS_7  = 1
} GbsStepCount;

typedef enum GbsWaveformLevel {
    GBS_WAVE_MUTE    = 0,
    GBS_WAVE_WHOLE   = 1,
    GBS_WAVE_HALF    = 2,
    GBS_WAVE_QUARTER = 3
} GbsWaveformLevel;

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

typedef struct GbsCh4Reg {
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

// ========================================================
// Synthesizer settings
// ========================================================

//
// Channel 1 settings
//
GbsErr gbs_ch1_setSweepTime(GbsSynth *synth, uint8_t ts);

GbsErr gbs_ch1_setSweepMode(GbsSynth *synth, GbsSweepMode mode);

GbsErr gbs_ch1_setSweepShift(GbsSynth *synth, uint8_t n);

#define gbs_ch1_setDuty(synth, duty) gbs_setDuty(synth, duty)

#define gbs_ch1_setLength(synth, length) gbs_setLength(synth, length)

#define gbs_ch1_setEnvSteps(synth, steps) gbs_setEnvSteps(synth, steps)

#define gbs_ch1_setEnvMode(synth, mode) gbs_setEnvMode(synth, mode)

#define gbs_ch1_setEnvLength(synth, length) gbs_setEnvLength(synth, length)

#define gbs_ch1_setFreq(synth, freq) gbs_setFreq(synth, freq)

//
// Channel 2 settings
//
#define gbs_ch2_setDuty(synth, duty) gbs_setDuty(synth, duty)

#define gbs_ch2_setLength(synth, length) gbs_setLength(synth, length)

#define gbs_ch2_setEnvSteps(synth, steps) gbs_setEnvSteps(synth, steps)

#define gbs_ch2_setEnvMode(synth, mode) gbs_setEnvMode(synth, mode)

#define gbs_ch2_setEnvLength(synth, length) gbs_setEnvLength(synth, length)

#define gbs_ch2_setFreq(synth, freq) gbs_setFreq(synth, freq)

//
// Channel 3 settings
//
GbsErr gbs_ch3_setEnable(GbsSynth *synth, bool enabled);

GbsErr gbs_ch3_setWaveform(GbsSynth *synth, uint8_t waveform[16]);

GbsErr gbs_ch3_setWaveformLevel(GbsSynth *synth, GbsWaveformLevel level);

#define gbs_ch3_setLength(synth, length) gbs_setLength(synth, length)

#define gbs_ch3_setFreq(synth, freq) gbs_setFreq(synth, freq)

//
// Channel 4 settings
//
GbsErr gbs_ch4_setScf(GbsSynth *synth, uint8_t scf);

GbsErr gbs_ch4_setStep(GbsSynth *synth, GbsStepCount steps);

GbsErr gbs_ch4_setDrf(GbsSynth *synth, uint8_t drf);

#define gbs_ch4_setLength(synth, length) gbs_setLength(synth, length)

#define gbs_ch4_setEnvSteps(synth, steps) gbs_setEnvSteps(synth, steps)

#define gbs_ch4_setEnvMode(synth, mode) gbs_setEnvMode(synth, mode)

#define gbs_ch4_setEnvLength(synth, length) gbs_setEnvLength(synth, length)

//
// general settings
//

//
// Set the waveform duty for the rectangle waveform generator.
// Channels: 1, 2
//
GbsErr gbs_setDuty(GbsSynth *synth, GbsDuty duty);

//
// Sets the length that the synthesizer will output sound when started
// This value is ignored if continuous output is set
// Channels: 1, 2, 3, 4
//
GbsErr gbs_setLength(GbsSynth *synth, uint8_t length);

//
// Sets the current step of the envelope function. Must be an integer between
// 0 and GBS_MAX_ENV_STEPS.
// Channels: 1, 2, 4
//
GbsErr gbs_setEnvSteps(GbsSynth *synth, uint8_t steps);

//
// Sets the mode of the envelope function
// Channels: 1, 2, 4
//
GbsErr gbs_setEnvMode(GbsSynth *synth, GbsEnvMode mode);

//
// Sets the length of time a step in the envelope takes. Must be an integer
// between 0 and GBS_MAX_ENV_LENGTH. The duration, in seconds, is determined by
// the formula: length / 64. If length is 0, then the envelope function does not occur. 
//
GbsErr gbs_setEnvLength(GbsSynth *synth, uint8_t length);

GbsErr gbs_setFreq(GbsSynth *synth, uint16_t freq);


// ===========================================================================
// Synthesizer Control
// ===========================================================================

//
// Initialize a synth handle with the given channel and sampling rate. If successful,
// the synth pointer will be set to a newly allocated GbsSynth struct, and is initialized
// with default settings.
//
GbsErr gbs_init(GbsSynth **synth, GbsChType channel, float samplingRate);

//
// Free any resources used by the given synth handle.
//
GbsErr gbs_deinit(GbsSynth *synth);

//
// Get the current sample of the synthesizer
//
float gbs_currentSample(GbsSynth *synth);

//
// Encode the current settings of the synthesizer into the register union.
// See the gameboy manual for the format of these registers
//
GbsErr gbs_getRegisters(GbsSynth* synth, GbsChRegUnion* reg);

//
// Simulate a single step of the synthesizer. A step is 1 sample of output. For example,
// if the synthesizer sampling rate is 44100 Hz, it will take 44100 ticks to output
// 1 second of sound.
//
GbsErr gbs_tick(GbsSynth *synth);

//
// utility functions
//

//
// Convert a 11-bit gameboy frequency to hertz, as a float. If the given freq is larger than
// the maximum, GBS_MAX_FREQUENCY, the maximum will be converted instead.
//
float gbs_freq(uint16_t freq);

//
// Inverse of gbs_freq. Converts a frequency, in hertz, to an 11-bit gameboy frequency.
//
uint16_t gbs_arcfreq(float freq);