
#include "trackerboy/synth.hpp"


namespace trackerboy {

static const float SAMPLE_TABLE[16] = {
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


Channel::Channel() :
    lengthCounter(DEFAULT_LENGTH),
    currentSample(SAMPLE_MIN),
    length(DEFAULT_LENGTH),
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