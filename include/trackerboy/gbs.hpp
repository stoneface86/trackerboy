/*
** File: include/trackerboy/gbs.hpp
**
** Contains a bunch of constants pertaining to the gameboy sound hardware
*/

#pragma once
#include <cstdint>
#include <cstddef>

#ifdef _MSC_VER
// disable warnings about unscoped enums
// as they have scope only in the Gbs namespace
#pragma warning( disable : 26812 )
#endif

namespace trackerboy {


namespace Gbs {

enum Duty : uint8_t {
    DUTY_125 = 0,
    DUTY_25 = 1,
    DUTY_50 = 2,
    DUTY_75 = 3
};

enum EnvMode : uint8_t {
    ENV_ATTENUATE = 0,
    ENV_AMPLIFY = 1
};

enum SweepMode : uint8_t {
    SWEEP_ADDITION = 0,
    SWEEP_SUBTRACTION = 1
};

enum NoiseSteps : uint8_t {
    NOISE_STEPS_15 = 0,
    NOISE_STEPS_7 = 1
};

enum WaveVolume : uint8_t {
    WAVE_MUTE = 0,
    WAVE_FULL = 1,
    WAVE_HALF = 2,
    WAVE_QUARTER = 3
};

enum OutputFlags : uint8_t {
    OUT_LEFT1 = 0x1,
    OUT_LEFT2 = 0x2,
    OUT_LEFT3 = 0x4,
    OUT_LEFT4 = 0x8,
    OUT_RIGHT1 = 0x10,
    OUT_RIGHT2 = 0x20,
    OUT_RIGHT3 = 0x40,
    OUT_RIGHT4 = 0x80,
    OUT_BOTH1 = OUT_LEFT1 | OUT_RIGHT1,
    OUT_BOTH2 = OUT_LEFT2 | OUT_RIGHT2,
    OUT_BOTH3 = OUT_LEFT3 | OUT_RIGHT3,
    OUT_BOTH4 = OUT_LEFT4 | OUT_RIGHT4,
    OUT_ALL = 0xFF,
    OUT_OFF = 0x0
};

enum Terminal : uint8_t {
    TERM_LEFT = 0x1,
    TERM_RIGHT = 0x2,
    TERM_BOTH = TERM_LEFT | TERM_RIGHT
};


// constants
static constexpr uint8_t MAX_ENV_STEPS          = 0xF;
static constexpr uint8_t MAX_ENV_LENGTH         = 0x7;
static constexpr uint16_t MAX_FREQUENCY         = 0x7FF;
static constexpr uint8_t MAX_LENGTH             = 0x3F;
static constexpr uint8_t MAX_LENGTH_WAVE        = 0xFF;
static constexpr uint8_t MAX_SCF                = 0xD;
static constexpr uint8_t MAX_SWEEP_TIME         = 0x7;
static constexpr uint8_t MAX_SWEEP_SHIFT        = 0x7;
static constexpr uint8_t MAX_TERM_VOLUME        = 0x7;

// default settings for the synthesizer
static constexpr uint8_t DEFAULT_DRF            = 0;
static constexpr Duty DEFAULT_DUTY              = DUTY_75;
static constexpr uint8_t DEFAULT_ENV_STEPS      = 0;
static constexpr uint8_t DEFAULT_ENV_LENGTH     = 0;
static constexpr EnvMode DEFAULT_ENV_MODE       = ENV_ATTENUATE;
static constexpr uint16_t DEFAULT_FREQUENCY     = 0;
static constexpr uint8_t DEFAULT_LENGTH         = 0;
static constexpr OutputFlags DEFAULT_OUTPUT     = OUT_OFF;
static constexpr uint8_t DEFAULT_SCF            = 0;
static constexpr NoiseSteps DEFAULT_STEP_COUNT  = NOISE_STEPS_15;
static constexpr uint8_t DEFAULT_SWEEP_TIME     = MAX_SWEEP_TIME;
static constexpr SweepMode DEFAULT_SWEEP_MODE   = SWEEP_ADDITION;
static constexpr uint8_t DEFAULT_SWEEP_SHIFT    = 0;
static constexpr uint8_t DEFAULT_TERM_VOLUME    = MAX_TERM_VOLUME;
static constexpr bool DEFAULT_TERM_ENABLE       = false;
static constexpr WaveVolume DEFAULT_WAVE_LEVEL  = WAVE_FULL;

static constexpr uint8_t SAMPLE_MAX             = 0xF;
static constexpr uint8_t SAMPLE_MIN             = 0x0;
static constexpr size_t WAVE_SIZE               = 32;
static constexpr size_t WAVE_RAMSIZE            = 16;

static constexpr float CLOCK_SPEED              = 4194304.0f;

}



}
