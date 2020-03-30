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
#pragma warning(disable : 26812)
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

enum Reg : uint16_t {
    // CH1 - Square 1 --------------------------------------------------------
    REG_NR10        = 0xFF10, // -PPP NSSS | sweep period, negate, shift
    REG_NR11        = 0xFF11, // DDLL LLLL | duty, length
    REG_NR12        = 0xFF12, // VVVV APPP | envelope volume, mode, period
    REG_NR13        = 0xFF13, // FFFF FFFF | Frequency LSB
    REG_NR14        = 0xFF14, // TL-- -FFF | Trigger, length enable, freq MSB
    // CH2 - Square 2 --------------------------------------------------------
    REG_UNUSED1     = 0xFF15,
    REG_NR21        = 0xFF16, // DDLL LLLL | duty, length
    REG_NR22        = 0xFF17, // VVVV APPP | envelope volume, mode, period
    REG_NR23        = 0xFF18, // FFFF FFFF | frequency LSB
    REG_NR24        = 0xFF19, // TL-- -FFF | Trigger, length enable, freq MSB
    // CH3 - Wave ------------------------------------------------------------
    REG_NR30        = 0xFF1A, // E--- ---- | DAC Power
    REG_NR31        = 0xFF1B, // LLLL LLLL | length
    REG_NR32        = 0xFF1C, // -VV- ---- | wave volume 
    REG_NR33        = 0xFF1D, // FFFF FFFF | frequency LSB
    REG_NR34        = 0xFF1E, // TL-- -FFF | Trigger, length enable, freq MSB
    // CH4 - Noise -----------------------------------------------------------
    REG_UNUSED2     = 0xFF1F,
    REG_NR41        = 0xFF20, // --LL LLLL | length
    REG_NR42        = 0xFF21, // VVVV APPP | envelope volume, mode, period 
    REG_NR43        = 0xFF22, // SSSS WDDD | clock shift, width, divisor mode
    REG_NR44        = 0xFF23, // TL-- ---- | trigger, length enable
    // Control/Status --------------------------------------------------------
    REG_NR50        = 0xFF24, // ALLL BRRR | Terminal enable/volume
    REG_NR51        = 0xFF25, // 4321 4321 | channel terminal enables
    REG_NR52        = 0xFF26, // P--- 4321 | Power control, channel len. stat
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
static constexpr uint8_t DEFAULT_ENV_REGISTER   = (DEFAULT_ENV_STEPS << 4) | 
                                                  (static_cast<uint8_t>(DEFAULT_ENV_MODE) << 3) |
                                                  (DEFAULT_ENV_LENGTH);
static constexpr uint16_t DEFAULT_FREQUENCY     = 0;
static constexpr uint8_t DEFAULT_LENGTH         = 0;
static constexpr OutputFlags DEFAULT_OUTPUT     = OUT_OFF;
static constexpr uint8_t DEFAULT_SCF            = 0;
static constexpr NoiseSteps DEFAULT_STEP_COUNT  = NOISE_STEPS_15;
static constexpr uint8_t DEFAULT_SWEEP_TIME     = MAX_SWEEP_TIME;
static constexpr SweepMode DEFAULT_SWEEP_MODE   = SWEEP_ADDITION;
static constexpr uint8_t DEFAULT_SWEEP_SHIFT    = 0;
static constexpr uint8_t DEFAULT_SWEEP_REGISTER = (DEFAULT_SWEEP_TIME << 4) |
                                                  (static_cast<uint8_t>(DEFAULT_SWEEP_MODE) << 3) |
                                                  (DEFAULT_SWEEP_SHIFT);
static constexpr uint8_t DEFAULT_TERM_VOLUME    = MAX_TERM_VOLUME;
static constexpr bool DEFAULT_TERM_ENABLE       = false;
static constexpr WaveVolume DEFAULT_WAVE_LEVEL  = WAVE_FULL;

static constexpr uint8_t DEFAULT_NOISE_REGISTER = (DEFAULT_SCF << 4) |
                                                  (DEFAULT_STEP_COUNT << 3) |
                                                  (DEFAULT_DRF);

static constexpr uint8_t SAMPLE_MAX             = 0xF;
static constexpr uint8_t SAMPLE_MIN             = 0x0;
static constexpr size_t WAVE_SIZE               = 32;
static constexpr size_t WAVE_RAMSIZE            = 16;

static constexpr float CLOCK_SPEED              = 4194304.0f;
static constexpr float FRAMERATE_GB             = 59.7f;    // vblank interrupt rate for DMG/CGB
static constexpr float FRAMERATE_SGB            = 61.1f;    // vblank interrupt rate for SGB

}



}
