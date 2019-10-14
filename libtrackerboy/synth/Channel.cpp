
#include "trackerboy/synth/Channel.hpp"
#include "trackerboy/gbs.hpp"

#include "sampletable.hpp"

#define calcFreqMax(f) ((2048 - f) * freqMultiplier)


namespace trackerboy {

//static const float SAMPLE_TABLE[16] = {
//    -1.0f,
//    -7.0f / 8.0f,
//    -6.0f / 8.0f,
//    -5.0f / 8.0f,
//    -4.0f / 8.0f,
//    -3.0f / 8.0f,
//    -2.0f / 8.0f,
//    -1.0f / 8.0f,
//    0.0f,
//    1.0f / 7.0f,
//    2.0f / 7.0f,
//    3.0f / 7.0f,
//    4.0f / 7.0f,
//    5.0f / 7.0f,
//    6.0f / 7.0f,
//    1.0f
//};


Channel::Channel(bool ch3) :
    lengthCounter(Gbs::DEFAULT_LENGTH),
    continuous(true),
    enabled(true),
    currentSample(Gbs::SAMPLE_MIN),
    length(Gbs::DEFAULT_LENGTH),
    frequency(Gbs::DEFAULT_FREQUENCY),
    freqMultiplier(ch3 ? 2 : 4),
    freqCounter(0),
    freqCounterMax(calcFreqMax(Gbs::DEFAULT_FREQUENCY)),
    sampleTable(SAMPLE_TABLE + (16 * Gbs::MAX_ENV_STEPS))
{
}

void Channel::disable() {
    enabled = false;
}

uint8_t Channel::getCurrentSample() {
    return enabled ? currentSample : Gbs::SAMPLE_MIN;
}

int16_t Channel::getCurrentVolume() {
    // convert current sample to a value between 0.0f and 1.0f
    //return enabled ? SAMPLE_TABLE[currentSample] : 0.0f;
    return enabled ? sampleTable[currentSample] : 0;
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
    currentSample = Gbs::SAMPLE_MIN;
}

void Channel::setContinuousOutput(bool _continuous) {
    continuous = _continuous;
}

void Channel::setFrequency(uint16_t _frequency) {
    if (_frequency > Gbs::MAX_FREQUENCY) {
        _frequency = Gbs::MAX_FREQUENCY;
    }

    frequency = _frequency;
    freqCounterMax = calcFreqMax(frequency);
}

void Channel::setLength(uint8_t _length) {
    if (_length > Gbs::MAX_LENGTH) {
        _length = Gbs::MAX_LENGTH;
    }
    length = _length;
}

}