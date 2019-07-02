
#include "gbsynth.h"


namespace gbsynth {

    FreqChannel::FreqChannel() {
        frequency = DEFAULT_FREQUENCY;
    }

    void FreqChannel::setFrequency(uint16_t frequency) {
        this->frequency = frequency;
        frequencyChanged();
    }

    void FreqChannel::setFrequency(float frequency) {
        this->frequency = toGbFreq(frequency);
        frequencyChanged();
    }

    void FreqChannel::frequencyChanged() {
        // do nothing
    }

}