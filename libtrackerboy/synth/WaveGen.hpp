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

#include "trackerboy/data/Waveform.hpp"
#include "synth/Generator.hpp"

namespace trackerboy {

class WaveGen : public Generator {

public:

    WaveGen() noexcept;

    void copyWave(Waveform &wave) noexcept;

    uint16_t frequency() const noexcept;

    void reset() noexcept override;

    void restart() noexcept override;

    void setFrequency(uint16_t frequency) noexcept;

    void setVolume(Gbs::WaveVolume volume) noexcept;

    void step(uint32_t cycles) noexcept;

    Gbs::WaveVolume volume() const noexcept;

private:

    uint16_t mFrequency;
    Gbs::WaveVolume mVolume;
    uint8_t mWaveIndex;
    uint8_t mWaveram[Gbs::WAVE_RAMSIZE];


};

}
