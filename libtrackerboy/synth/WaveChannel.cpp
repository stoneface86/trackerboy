
#include "trackerboy/synth/WaveChannel.hpp"

#include <algorithm>

namespace trackerboy {

WaveChannel::WaveChannel() :
    Channel(true),
    outputLevel(Gbs::DEFAULT_WAVE_LEVEL),
    mWaveform(),
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

void WaveChannel::setWaveform(const Waveform &waveform) {
    std::copy_n(waveform.data, Gbs::WAVE_RAMSIZE, mWaveform.data);
}

void WaveChannel::setWaveform(const uint8_t waveform[Gbs::WAVE_RAMSIZE]) {
    std::copy_n(waveform, Gbs::WAVE_RAMSIZE, mWaveform.data);
}

void WaveChannel::step(unsigned cycles) {
    freqCounter += cycles;
    unsigned wavesteps = freqCounter / freqCounterMax;
    freqCounter %= freqCounterMax;
    waveIndex = (waveIndex + wavesteps) & 0x1F; // & 0x1F == % 32
    uint8_t sample = mWaveform.data[waveIndex >> 1];
    if (waveIndex & 1) {
        // odd number, low nibble
        sample &= 0xF;
    } else {
        // even number, high nibble
        sample >>= 4;
    }
    // convert the sample with bias of -SAMPLE_GND
    //int8_t biased = static_cast<int8_t>(sample) - Gbs::SAMPLE_GND;
    switch (outputLevel) {
        case Gbs::WAVE_MUTE:
            sample = Gbs::SAMPLE_MIN;
            break;
        case Gbs::WAVE_FULL:
            break; // nothing to do
        case Gbs::WAVE_HALF:
            sample >>= 1;
            break;
        case Gbs::WAVE_QUARTER:
            // same as WAVE_HALF except shift by 2
            sample >>= 2;
            break;
    }

    currentSample = sample;
}

}