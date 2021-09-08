/*
** Trackerboy - Gameboy / Gameboy Color music tracker
** Copyright (C) 2019-2021 stoneface86
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

#include "trackerboy/trackerboy.hpp"

#include <cstdint>

namespace trackerboy {

//
// Structure representing the current state of a channel.
// Register writes are only applied to changes in state
//
struct ChannelState {

    static constexpr uint8_t defaultEnvelope(ChType ch) {
        if (ch == ChType::ch3) {
            return 0;
        } else {
            return 0xF0;
        }
    }

    static constexpr uint8_t defaultTimbre(ChType ch) {
        if (ch == ChType::ch4) {
            return 0;
        } else {
            return 3;
        }
    }

    static constexpr uint8_t defaultPanning(ChType ch) {
        (void)ch;
        return 3;
    }

    constexpr ChannelState() :
        playing(false),
        retrigger(false),
        envelope(0),
        timbre(0),
        panning(0),
        frequency(0)
    {
    }

    // constructs the default state for the given channel
    constexpr ChannelState(ChType ch) :
        playing(false),
        retrigger(false),
        envelope(defaultEnvelope(ch)),
        timbre(defaultTimbre(ch)),
        panning(defaultPanning(ch)),
        frequency(0)
    {
    }

    constexpr ChannelState(bool playing, uint8_t envelope, uint8_t timbre, uint8_t panning, uint16_t frequency) :
        playing(playing),
        retrigger(false),
        envelope(envelope),
        timbre(timbre),
        panning(panning),
        frequency(frequency)
    {
    }

    // determines if a note is playing, or if the DAC is on
    bool playing;
    bool retrigger;     // retrigger override
    uint8_t envelope;   // volume envelope or waveform id for CH3
    uint8_t timbre;     // duty/volume/stepwidth setting
    uint8_t panning;    // channel panning setting (0: mute, 1: left, 2: right, 3: middle)
    uint16_t frequency; // channel frequency

};

}
