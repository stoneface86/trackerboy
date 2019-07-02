
#include "gbsynth.h"

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
    }

    void WaveChannel::setOutputLevel(WaveformLevel level) {
        outputLevel = level;
    }

    void WaveChannel::setWaveform(uint8_t waveform[WAVE_RAMSIZE]) {
        copy_n(waveform, WAVE_RAMSIZE, wavedata);
    }

    uint8_t WaveChannel::generate(unsigned cycles) {
        
        return 0;
    }

}