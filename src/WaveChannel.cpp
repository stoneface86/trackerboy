
#include "gbsynth.hpp"

#include <algorithm>

namespace gbsynth {

WaveChannel::WaveChannel() :
    Channel(),
    FreqChannel(),
    wavedata{0},
    outputLevel(static_cast<WaveVolume>(DEFAULT_WAVE_LEVEL)),
    waveIndex(0)
{
}

void WaveChannel::reset() {
    waveIndex = 0;
    freqCounter = 0;
}

void WaveChannel::setOutputLevel(WaveVolume level) {
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
        case WaveVolume::mute:
            sample = SAMPLE_GND;
        case WaveVolume::full:
            break; // nothing to do
        case WaveVolume::half:
            sample >>= 1;
        case WaveVolume::quarter:
            sample >>= 2;
    }

    currentSample = sample;
}

}