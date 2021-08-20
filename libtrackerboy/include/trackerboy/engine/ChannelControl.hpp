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

#include "trackerboy/engine/RuntimeContext.hpp"
#include "trackerboy/engine/ChannelState.hpp"
#include "trackerboy/engine/IApu.hpp"
#include "trackerboy/trackerboy.hpp"

#include <array>
#include <bitset>
#include <cstdint>

namespace trackerboy {

//
// Class handles all APU register writes
//
template <ChType ch>
class ChannelControl {

public:

    static void update(
        IApu &apu,
        WaveformTable const& waveTable,
        ChannelState const& lastState,
        ChannelState const& state
    ) noexcept;

    static void clear(IApu &apu) noexcept;

    static void init(IApu &apu, WaveformTable const& waveTable, ChannelState const& state) noexcept;

};

/*
class ChannelControl {

public:
    ChannelControl();

    bool isLocked(ChType ch) const noexcept;

    //uint8_t lockbits() const noexcept;

    void lock(RuntimeContext const& rc, ChType ch) noexcept;

    void unlock(RuntimeContext const& rc, ChType ch) noexcept;

    //
    // Update the given channel's state. If the channel is locked, the state is
    // written to the Apu's registers
    //
    void update(ChType ch, RuntimeContext const& rc, ChannelState const& state) noexcept;

    template <ChType ch>
    void update(gbapu::Apu &apu, WaveformList const& wavelist, ChannelState const& state) noexcept;

    //
    // Write the given envelope to the channel's registers. For channels 1, 2 and 4 this
    // value is written to the channel's envelope register (NRx2). For channel 3, the
    // waveram is set to the waveform in the wave table with the envelope value being the index.
    // The channel is then restarted.
    //
    static void writeEnvelope(ChType ch, RuntimeContext &rc, uint8_t envelope, uint8_t freqMsb = 0x0);

    static void writeFrequency(ChType ch, RuntimeContext &rc, uint16_t frequency);

    //
    // Write the given timbre to the channel's registers. A timbre ranges from 0-3 and its
    // effect depends on the channel.
    //
    // CH1, CH2: the duty is set (0 = 12.5%, 1 = 25%, 2 = 50%, 3 = 75%)
    // CH3:      the volume is set (0 = mute, 1 = 25%, 2 = 50%, 3 = 100%)
    // CH4:      the step-width is set (0 = 15-bit, 1,2,3 = 7-bit)
    //
    static void writeTimbre(ChType ch, RuntimeContext &rc, uint8_t timbre);

    //
    // Writes panning settings for a channel.
    //
    static void writePanning(ChType ch, RuntimeContext &rc, uint8_t panning);

    static void writeWaveram(RuntimeContext &rc, Waveform &waveform);

private:

    //
    // Bit 0: CH1 Lock status
    // Bit 1: CH2 Lock status
    // Bit 2: CH3 Lock status
    // Bit 3: CH4 Lock status
    // Bit 4: CH1 Sfx trigger status (TODO)
    // Bit 5: CH2 Sfx trigger status
    // Bit 6: CH3 Sfx trigger status
    // Bit 7: CH4 Sfx trigger status
    // 
    //
    //uint8_t mLocks;

    std::bitset<4> mLocks;

    std::array<ChannelState, 4> mChannelState;

};
*/

}
