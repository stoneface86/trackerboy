
#include "trackerboy/synth/WaveChannel.hpp"

#include <algorithm>

namespace trackerboy {

WaveChannel::WaveChannel() :
    Channel(true),
    mOutputLevel(Gbs::DEFAULT_WAVE_LEVEL),
    mWaveform(),
    mWaveIndex(0)
{
}

void WaveChannel::reset() {
    mWaveIndex = 0;
    mFreqCounter = 0;
}

void WaveChannel::setOutputLevel(Gbs::WaveVolume level) {
    mOutputLevel = level;
}

void WaveChannel::setWaveform(const Waveform &waveform) {
    std::copy_n(waveform.data, Gbs::WAVE_RAMSIZE, mWaveform.data);
}

void WaveChannel::setWaveform(const uint8_t waveform[Gbs::WAVE_RAMSIZE]) {
    std::copy_n(waveform, Gbs::WAVE_RAMSIZE, mWaveform.data);
}

void WaveChannel::step(unsigned cycles) {
    mFreqCounter += cycles;
    unsigned wavesteps = mFreqCounter / mFreqCounterMax;
    mFreqCounter %= mFreqCounterMax;
    mWaveIndex = (mWaveIndex + wavesteps) & 0x1F; // & 0x1F == % 32
    uint8_t sample = mWaveform.data[mWaveIndex >> 1];
    if (mWaveIndex & 1) {
        // odd number, low nibble
        sample &= 0xF;
    } else {
        // even number, high nibble
        sample >>= 4;
    }
    // convert the sample with bias of -SAMPLE_GND
    //int8_t biased = static_cast<int8_t>(sample) - Gbs::SAMPLE_GND;
    switch (mOutputLevel) {
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

    mCurrentSample = sample;
}

}