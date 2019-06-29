
#include "gbsynth.h"

#include <algorithm>

using std::fill_n;

namespace gbsynth {

    WaveChannel::WaveChannel(float samplingRate) :
        Channel(samplingRate)
    {
        fill_n(wavedata, 16, 0);
        frequency = DEFAULT_FREQUENCY;
        outputLevel = (WaveformLevel)DEFAULT_WAVE_LEVEL;
    }

    size_t WaveChannel::generate(uint8_t buf[], size_t nsamples) {
        osc.fill(buf, nsamples);
        return nsamples;
    }

    void WaveChannel::getRegisters(ChRegUnion &reg) {

    }

    void WaveChannel::setOutputLevel(WaveformLevel level) {
        outputLevel = level;
    }

    void WaveChannel::setFrequency(uint16_t frequency) {
        this->frequency = frequency;
        osc.setFrequency(samplingRate, fromGbFreq(frequency));
    }

    void WaveChannel::setWaveform(uint8_t waveform[WAVE_RAMSIZE]) {
        uint8_t buf[WAVE_SIZE];
        uint8_t pair;
        for (size_t i = 0; i != WAVE_RAMSIZE; ++i) {
            pair = waveform[i];
            buf[i] = pair >> 4;
            buf[i + 1] = pair & 0xF;
        }
        osc.setWaveform(buf, 32);
    }

    void WaveChannel::reset() {
        osc.reset();
    }



}