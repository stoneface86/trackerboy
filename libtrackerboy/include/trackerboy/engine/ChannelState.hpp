
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
