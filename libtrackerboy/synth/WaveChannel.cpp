
#include "trackerboy/synth/WaveChannel.hpp"

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

void WaveChannel::setWaveform(const uint8_t waveform[WAVE_RAMSIZE]) {
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
    // convert the sample with bias of -SAMPLE_GND
    int8_t biased = static_cast<int8_t>(sample) - Gbs::SAMPLE_GND;
    switch (outputLevel) {
        case Gbs::WAVE_MUTE:
            sample = Gbs::SAMPLE_GND;
            break;
        case Gbs::WAVE_FULL:
            break; // nothing to do
        case Gbs::WAVE_HALF:
            // shift the bias by 1 and set sample to the unbiased form
            sample = (biased >> 1) + Gbs::SAMPLE_GND;
            break;
        case Gbs::WAVE_QUARTER:
            // same as WAVE_HALF except shift by 2
            sample = (biased >> 2) + Gbs::SAMPLE_GND;
            break;
    }

    currentSample = sample;
}

}