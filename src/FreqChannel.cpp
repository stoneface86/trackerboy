
#include "gbsynth.hpp"

#define calcFreqMax(f) ((2048 - f) * multiplier)

namespace gbsynth {

    FreqChannel::FreqChannel(unsigned multiplier) :
        frequency(DEFAULT_FREQUENCY),
        freqCounter(0),
        freqCounterMax(calcFreqMax(frequency)),
        multiplier(multiplier)
    {
    }

    uint16_t FreqChannel::getFrequency() {
        return frequency;
    }

    void FreqChannel::setFrequency(uint16_t frequency) {
        this->frequency = frequency;
        freqCounterMax = calcFreqMax(frequency);
    }

    void FreqChannel::setFrequency(float frequency) {
        this->frequency = toGbFreq(frequency);
        freqCounterMax = calcFreqMax(this->frequency);
    }

}