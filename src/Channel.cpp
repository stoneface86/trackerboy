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
        return enabled ? currentSample : SAMPLE_MIN;
    }

    float Channel::getCurrentVolume() {
        // convert current sample to a value between 0.0f and 1.0f
        return enabled ? (currentSample / (float)SAMPLE_MAX) : 0.0f;
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
        if (!continuous) {
            if (lengthCounter == 0) {
                enabled = false;
            } else {
                --lengthCounter;
            }  
        }
    }

    void Channel::reset() {
        lengthCounter = length;
        enabled = true;
        currentSample = SAMPLE_MIN;
    }

    void Channel::step() {
        currentSample = generate();
    }

    void Channel::disable() {
        enabled = false;
    }

}