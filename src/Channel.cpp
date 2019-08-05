
#include "gbsynth.hpp"
#include "tables.h"


namespace gbsynth {

Channel::Channel() :
    lengthCounter(0),
    currentSample(SAMPLE_MIN),
    length(0),
    continuous(true),
    enabled(true)
{
}

void Channel::disable() {
    enabled = false;
}

uint8_t Channel::getCurrentSample() {
    return enabled ? currentSample : SAMPLE_GND;
}

float Channel::getCurrentVolume() {
    // convert current sample to a value between 0.0f and 1.0f
    return enabled ? SAMPLE_TABLE[currentSample] : 0.0f;
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

void Channel::setContinuousOutput(bool _continuous) {
    continuous = _continuous;
}

void Channel::setLength(uint8_t _length) {
    if (_length > MAX_LENGTH) {
        _length = MAX_LENGTH;
    }
    length = _length;
}

}