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

#include "trackerboy/trackerboy.hpp"
#include "trackerboy/data/DataItem.hpp"
#include "trackerboy/data/Sequence.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace trackerboy {


// New instrument format:
// - an envelope/waveform setting
// - 4 sequences that each modulate a parameter (panning, arp, pitch, timbre)


class Instrument : public DataItem {

public:

    static constexpr size_t SEQUENCE_ARP = 0;
    static constexpr size_t SEQUENCE_PANNING = 1;
    static constexpr size_t SEQUENCE_PITCH = 2;
    static constexpr size_t SEQUENCE_TIMBRE = 3;
    static constexpr size_t SEQUENCE_COUNT = 4;

    using SequenceArray = std::array<Sequence, SEQUENCE_COUNT>;


    Instrument();
    Instrument(Instrument const& instrument);

    ChType channel() const noexcept;

    bool hasEnvelope() const noexcept;

    uint8_t envelope() const noexcept;

    std::optional<uint8_t> queryEnvelope() const noexcept;

    SequenceArray& sequences() noexcept;
    SequenceArray const& sequences() const noexcept;

    Sequence::Enumerator enumerateSequence(size_t parameter) const noexcept;

    Sequence& sequence(size_t parameter) noexcept;
    Sequence const& sequence(size_t parameter) const noexcept;

    void setChannel(ChType ch) noexcept;

    void setEnvelope(uint8_t value) noexcept;

    void setEnvelopeEnable(bool enable) noexcept;


private:

    ChType mChannel;
    bool mEnvelopeEnabled;
    // volume envelope / waveform id
    uint8_t mEnvelope;

    // parameter sequences
    SequenceArray mSequences;


};




}
