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

#pragma once

#include <cstdint>

namespace trackerboy {

//
// Typedef for a note index. Music notes used by this tracker are in
// octaves 2 to 8, starting at octave 2. Note indices can be set by
// adding the octave and note constants. To get the index of G-5 you
// would set the value to OCTAVE_5 + NOTE_G
//
enum Note : uint8_t {
    // notes
    NOTE_C  = 0,                // C        (C-2)
    NOTE_Db = 1,                // Db/C#    (C#2)
    NOTE_D  = 2,                // D        (D-2)
    NOTE_Eb = 3,                // Eb/D#    (D#2)
    NOTE_E  = 4,                // E        (E-2)
    NOTE_F  = 5,                // F        (F-2)
    NOTE_Gb = 6,                // Gb/F#    (F#2)
    NOTE_G  = 7,                // G        (G-2)
    NOTE_Ab = 8,                // Ab/G#    (G#2)
    NOTE_A  = 9,                // A        (A-2)
    NOTE_Bb = 10,               // Bb/A#    (A#2)
    NOTE_B  = 11,               // B        (B-2)

    // octaves
    OCTAVE_2 = 12 * 0,          // C-2
    OCTAVE_3 = 12 * 1,          // C-3
    OCTAVE_4 = 12 * 2,          // C-4
    OCTAVE_5 = 12 * 3,          // C-5
    OCTAVE_6 = 12 * 4,          // C-6
    OCTAVE_7 = 12 * 5,          // C-7
    OCTAVE_8 = 12 * 6,          // C-8

    NOTE_NONE = 0xFF,           // null note index
    NOTE_LAST = (12 * 7) - 1,   // 7 octaves of notes possible, 2-8
    NOTE_NOISE_LAST = (12 * 5) - 1, // last possible note for CH4
    NOTE_CUT = NOTE_LAST + 1    // special index: stop playing the last note (stops instrument)
};

// note frequency lookup table
extern const uint16_t NOTE_FREQ_TABLE[NOTE_LAST + 1];

extern const uint8_t NOTE_NOISE_TABLE[NOTE_NOISE_LAST + 1];


}
