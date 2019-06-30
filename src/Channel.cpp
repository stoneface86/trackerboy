#include "gbsynth.h"

#include <cmath>


namespace gbsynth {

    Channel::Channel() {
        lengthCounter = 0;
        currentSample = SAMPLE_MIN;
        length = 0;
        continuous = true;
        enabled = true;
    }

    uint8_t Channel::getCurrentSample() {
        // TODO
        return 0;
    }

    float Channel::getCurrentVolume() {
        // TODO
        return 0.0f;
    }

    void Channel::setLength(uint8_t length) {
        if (length > MAX_LENGTH) {
            length = MAX_LENGTH;
        }
        this->length = length;
    }

    void Channel::setContinuousOutput(bool continuous) {
        this->continuous = continuous;
    }

    void Channel::lengthStep() {
        // TODO
    }

    void Channel::reset() {
        // TODO
    }
}