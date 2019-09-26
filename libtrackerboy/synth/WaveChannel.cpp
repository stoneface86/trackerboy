
#include "trackerboy/synth.hpp"

#include <algorithm>

namespace trackerboy {

WaveChannel::WaveChannel() :
    Channel(true),
    wavedata{0},
    outputLevel(Gbs::DEFAULT_WAVE_LEVEL),
    waveIndex(0)
{
}

void WaveChannel::reset() {
    waveIndex = 0;
    freqCounter = 0;
}

void WaveChannel::setOutputLevel(Gbs::WaveVolume level) {
    outputLevel = level;
}

void WaveChannel::setWaveform(uint8_t waveform[WAVE_RAMSIZE]) {
    std::copy_n(waveform, WAVE_RAMSIZE, wavedata);
}

void WaveChannel::step(unsigned cycles) {
    freqCounter += cycles;
    unsigned wavesteps = freqCounter / freqCounterMax;
    freqCounter %= freqCounterMax;
    waveIndex = (waveIndex + wavesteps) & 0x1F; // & 0x1F == % 32
    uint8_t sample = wavedata[waveIndex >> 1];
    if (waveIndex & 1) {
        // odd number, low nibble
        sample &= 0xF;
    } else {
        // even number, high nibble
        sample >>= 4;
    }
    switch (outputLevel) {
        case Gbs::WAVE_MUTE:
            sample = Gbs::SAMPLE_GND;
        case Gbs::WAVE_FULL:
            break; // nothing to do
        case Gbs::WAVE_HALF:
            sample >>= 1;
        case Gbs::WAVE_QUARTER:
            sample >>= 2;
    }

    currentSample = sample;
}

}