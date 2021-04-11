/*
** Trackerboy - Gameboy / Gameboy Color music tracker
** Copyright (C) 2019-2020 stoneface86
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
*/

// common typedefs and constants used throughout the library

#pragma once

#include <type_traits>
#include <cstdint>

namespace trackerboy {

enum class System : uint8_t {
    dmg,
    sgb
};

//
// Channel enum
//
enum class ChType : uint8_t {
    ch1 = 0,
    ch2 = 1,
    ch3 = 2,
    ch4 = 3
};

enum class EffectType : uint8_t {

    // A * indicates the effect is continuous and must be turned off (ie 400)

    // pattern effect (bits 6-7 = 00)
    patternGoto,                            //   Bxx begin playing given pattern immediately
    patternHalt,                            //   C00 stop playing
    patternSkip,                            //   D00 begin playing next pattern immediately
    setTempo,                               //   Fxx set the tempo
    sfx,                                    // * Txx play sound effect

    // track effect (bits 6-7 = 01)

    setEnvelope,                            //   Exx set the persistent envelope/wave id setting
    setTimbre,                              //   Vxx set persistent duty/wave volume setting
    setPanning,                             //   Ixy set channel panning setting
    setSweep,                               //   Hxx set the persistent sweep setting (CH1 only)
    delayedCut,                             //   Sxx note cut delayed by xx frames
    delayedNote,                            //   Gxx note trigger delayed by xx frames
    lock,                                   //   L00 (lock) stop the sound effect on the current channel


    // frequency effect (bits 6-7 = 10)
    arpeggio,                               // * 0xy arpeggio with semi tones x and y
    pitchUp,                                // * 1xx pitch slide up
    pitchDown,                              // * 2xx pitch slide down
    autoPortamento,                         // * 3xx automatic portamento
    vibrato,                                // * 4xy vibrato
    vibratoDelay,                           //   5xx delay vibrato xx frames on note trigger
    tuning,                                 //   Pxx fine tuning
    noteSlideUp,                            // * Qxy note slide up
    noteSlideDown                           // * Rxy note slide down

};

enum class InstrumentType {
    pulse,
    wave,
    noise
};

enum class Panning : uint8_t {
    mute,
    left,
    right,
    middle
};

//
// The speed type determines the tempo during pattern playback. Its unit is
// frames per row in Q4.4 format. Speeds with a fractional component will
// have some rows taking an extra frame.
//
using Speed = uint8_t;

// minimum possible speed, 1.0 frames per row
static constexpr Speed SPEED_MIN = 0x10;

// maximum possible speed, 15.0 frames per row
static constexpr Speed SPEED_MAX = 0xF0;

constexpr size_t MAX_INSTRUMENTS = 256;
constexpr size_t MAX_WAVEFORMS = 256;
constexpr size_t MAX_PATTERNS = 256;

template <typename T>
constexpr T GB_CLOCK_SPEED = T(4194304);

constexpr float GB_FRAMERATE_DMG = 59.7f;
constexpr float GB_FRAMERATE_SGB = 61.1f;

// each channel has 5 registers
constexpr unsigned GB_CHANNEL_REGS = 5;

//
// Maximum frequency setting for channels 1, 2 and 3
//
constexpr uint16_t GB_MAX_FREQUENCY = 2047;

//
// CH3 waveram is 16 bytes
//
constexpr size_t GB_WAVERAM_SIZE = 16;




}
