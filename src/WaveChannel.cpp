
#include "gbsynth.hpp"

#include <algorithm>

using std::copy_n;

namespace gbsynth {

    WaveChannel::WaveChannel() :
        Channel(),
        FreqChannel(2),
        wavedata{0},
        outputLevel((WaveformLevel)DEFAULT_WAVE_LEVEL),
        waveIndex(0)
    {
    }

    void WaveChannel::reset() {
        waveIndex = 0;
        freqCounter = 0;
    }

    void WaveChannel::setOutputLevel(WaveformLevel level) {
        outputLevel = level;
    }

    void WaveChannel::setWaveform(uint8_t waveform[WAVE_RAMSIZE]) {
        copy_n(waveform, WAVE_RAMSIZE, wavedata);
    }

    uint8_t WaveChannel::generate(unsigned cycles) {
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
            case WAVE_MUTE:
                return SAMPLE_GND;
            case WAVE_WHOLE:
                return sample;
            case WAVE_HALF:
                return sample >> 1;
            case WAVE_QUARTER:
                return sample >> 2;
            default:
                return SAMPLE_GND; // should never happen
        }
    }

}