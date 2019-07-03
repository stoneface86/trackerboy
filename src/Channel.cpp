#include "gbsynth.h"

#include <cmath>


namespace gbsynth {

    static const float SAMPLE_TABLE[] = {
        -1.0f,
        -7.0f / 8.0f,
        -6.0f / 8.0f,
        -5.0f / 8.0f,
        -4.0f / 8.0f,
        -3.0f / 8.0f,
        -2.0f / 8.0f,
        -1.0f / 8.0f,
         0.0f,
         1.0f / 7.0f,
         2.0f / 7.0f,
         3.0f / 7.0f,
         4.0f / 7.0f,
         5.0f / 7.0f,
         6.0f / 7.0f,
         1.0f
    };

    Channel::Channel() {
        lengthCounter = 0;
        currentSample = SAMPLE_GND;
        length = 0;
        continuous = true;
        enabled = true;
    }

    uint8_t Channel::getCurrentSample() {
        return enabled ? currentSample : SAMPLE_GND;
    }

    float Channel::getCurrentVolume() {
        // convert current sample to a value between 0.0f and 1.0f
        return enabled ? SAMPLE_TABLE[currentSample] : 0.0f;
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
        currentSample = SAMPLE_GND;
    }

    void Channel::step(unsigned cycles) {
        currentSample = generate(cycles);
    }

    void Channel::disable() {
        enabled = false;
    }

}