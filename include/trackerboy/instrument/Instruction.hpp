
#pragma once

#include <cstdint>


namespace trackerboy {

//
// POD struct for an instrument instruction. An instruction determines
// which settings of the channel to set. Instructions are 3 bytes long.
//
struct Instruction {
    uint8_t duration;    // number of frames this instruction runs for
    uint8_t ctrl;        // control flags
    uint8_t settings;    // settings (channel specific)
    uint8_t envSettings; // envelope settings (except for channel 3)
    uint8_t note;        // note/frequency to set (channels 1, 2, 3)

    //
    // Bit masks for all fields in the ctrl byte
    //
    enum CtrlFlags {
        CTRL_DUTY = 0x03, // duty setting bits 0-1 (channels 1,2)
        CTRL_SET_WAVE = 0x03, // wave set flag bits 0-1 (channel 3)
        CTRL_SET_WAVEVOL = 0x04, // wave volume set flag bit 2 (channel 3)
        CTRL_SET_NOISE = 0x01, // noise set flag bit 0 (channel 4)
        CTRL_SET_DUTY = 0x04, // duty set flag bit 2 (channels 1,2)
        CTRL_SET_ENV = 0x18, // envelope set flag bits 3-4 (channels 1,2,4)
        CTRL_PANNING = 0x60, // panning setting bits 5-6 (all channels)
        CTRL_INIT = 0x80, // init flag bit 7 (all channels)
    };

    enum SettingsFlags {
        SETTINGS_SET_SWEEP = 0x80, // sweep set flag bit 7 (channel 1 only)
        SETTINGS_WAVEVOL = 0x03  // wave volume settings bits 0-1 (channel 3 only)
    };

    //
    // Panning control flags
    //
    enum Panning {
        PANNING_NOSET = 0x00, // panning remains unchanged
        PANNING_LEFT = 0x20, // channel panning is set to left
        PANNING_RIGHT = 0x40, // channel panning is set to right
        PANNING_BOTH = 0x60, // channel panning is set to left and right
    };

    //
    // Duty setting values
    //
    enum Duty {
        DUTY_125 = 0x00,
        DUTY_25 = 0x01,
        DUTY_50 = 0x02,
        DUTY_75 = 0x03
    };

    //
    // Envelope control flags, determines if the envelope will be set
    //
    enum EnvCtrl {
        ENV_NOSET = 0x00, // envelope remains unchanged
        ENV_SETVOL = 0x08, // envelope is set with volume in instruction
        ENV_SETNOVOL = 0x10  // envelope is set with volume from note
    };

    enum WaveCtrl {
        WAVE_NOSET = 0x00, // do not set wave ram
        WAVE_SET = 0x01, // set wave ram
        WAVE_SETLONG = 0x02  // long play wave
    };

    enum WaveVol {
        WAVE_VOL_MUTE = 0x00,
        WAVE_VOL_FULL = 0x01,
        WAVE_VOL_HALF = 0x02,
        WAVE_VOL_QUARTER = 0x03
    };
};

}
