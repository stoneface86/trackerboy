
#include <algorithm>

#include "synth/WaveGen.hpp"


namespace {

// multiplier for frequency calculation
// 32 Hz - 65.536 KHz
static constexpr unsigned WAVE_MULTIPLIER = 2;

static constexpr uint32_t DEFAULT_PERIOD = (2048 - trackerboy::Gbs::DEFAULT_FREQUENCY) * WAVE_MULTIPLIER;

}

namespace trackerboy {

WaveGen::WaveGen() noexcept :
    Generator(DEFAULT_PERIOD, 0),
    mFrequency(Gbs::DEFAULT_FREQUENCY),
    mVolume(Gbs::DEFAULT_WAVE_LEVEL),
    mWaveIndex(0),
    mWaveram{0}
{
}

void WaveGen::copyWave(Waveform &wave) noexcept {
    std::copy_n(wave.data(), Gbs::WAVE_RAMSIZE, mWaveram);
}

uint16_t WaveGen::frequency() const noexcept {
    return mFrequency;
}

void WaveGen::reset() noexcept {
    mFrequency = Gbs::DEFAULT_FREQUENCY;
    mVolume = Gbs::DEFAULT_WAVE_LEVEL;
    std::fill_n(mWaveram, Gbs::WAVE_RAMSIZE, static_cast<uint8_t>(0));
    restart();
}

void WaveGen::restart() noexcept {
    Generator::restart();
    mWaveIndex = 0;
}

void WaveGen::setFrequency(uint16_t frequency) noexcept {
    mFrequency = frequency;
    mPeriod = (2048 - frequency) * WAVE_MULTIPLIER;
}

void WaveGen::setVolume(Gbs::WaveVolume volume) noexcept {
    mVolume = volume;
}

void WaveGen::step(uint32_t cycles) noexcept {
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

Gbs::WaveVolume WaveGen::volume() const noexcept {
    return mVolume;
}


}
