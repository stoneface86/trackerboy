
#include <algorithm>

#include "trackerboy/synth/WaveGen.hpp"


namespace {

// multiplier for frequency calculation
// 32 Hz - 65.536 KHz
static constexpr unsigned WAVE_MULTIPLIER = 2;

static constexpr uint32_t DEFAULT_PERIOD = (2048 - trackerboy::Gbs::DEFAULT_FREQUENCY) * WAVE_MULTIPLIER;

}

namespace trackerboy {

WaveGen::WaveGen() :
    Generator(DEFAULT_PERIOD, 0),
    mVolume(Gbs::DEFAULT_WAVE_LEVEL),
    mWaveIndex(0),
    mWaveram{0}
{
}

void WaveGen::copyWave(Waveform &wave) {
    std::copy_n(wave.data(), Gbs::WAVE_RAMSIZE, mWaveram);
}

void WaveGen::restart() {
    Generator::restart();
    mWaveIndex = 0;
}

void WaveGen::setFrequency(uint16_t frequency) {
    mPeriod = (2048 - frequency) * WAVE_MULTIPLIER;
}

void WaveGen::setVolume(Gbs::WaveVolume volume) {
    mVolume = volume;
}

void WaveGen::step(uint32_t cycles) {
    mFreqCounter += cycles;
    uint32_t wavesteps = mFreqCounter / mPeriod;
    mFreqCounter %= mPeriod;
    mWaveIndex = (mWaveIndex + wavesteps) & 0x1F; // & 0x1F == % 32

    uint8_t sample = mWaveram[mWaveIndex >> 1];
    if (mWaveIndex & 1) {
        // odd number, low nibble
        sample &= 0xF;
    } else {
        // even number, high nibble
        sample >>= 4;
    }

    switch (mVolume) {
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

    mOutput = sample;
}


}
