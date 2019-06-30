
#include "gbsynth.h"

#include <algorithm>

using std::fill_n;

namespace gbsynth {

    WaveChannel::WaveChannel() : Channel(), FreqChannel() {
        fill_n(wavedata, 16, 0);
        outputLevel = (WaveformLevel)DEFAULT_WAVE_LEVEL;
    }

    void WaveChannel::reset() {
        // TODO
    }

    void WaveChannel::setOutputLevel(WaveformLevel level) {
        outputLevel = level;
    }

    void WaveChannel::setWaveform(uint8_t waveform[WAVE_RAMSIZE]) {
        // TODO
    }

    uint8_t WaveChannel::generate() {
        // TODO
        return 0;
    }

}