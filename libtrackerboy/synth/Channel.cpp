
#include "trackerboy/synth/Channel.hpp"
#include "trackerboy/gbs.hpp"

#include "sampletable.hpp"

#define calcFreqMax(f) ((2048 - f) * mFreqMultiplier)


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
    mLengthCounter(Gbs::DEFAULT_LENGTH),
    mContinuous(true),
    mEnabled(true),
    mCurrentSample(Gbs::SAMPLE_MIN),
    mLength(Gbs::DEFAULT_LENGTH),
    mFrequency(Gbs::DEFAULT_FREQUENCY),
    mFreqMultiplier(ch3 ? 2 : 4),
    mFreqCounter(0),
    mFreqCounterMax(calcFreqMax(Gbs::DEFAULT_FREQUENCY)),
    mSampleTable(SAMPLE_TABLE + (16 * Gbs::MAX_ENV_STEPS))
{
}

void Channel::disable() {
    mEnabled = false;
}

uint8_t Channel::getCurrentSample() {
    return mEnabled ? mCurrentSample : Gbs::SAMPLE_MIN;
}

int16_t Channel::getCurrentVolume() {
    // convert current sample to a value between 0.0f and 1.0f
    //return enabled ? SAMPLE_TABLE[currentSample] : 0.0f;
    return mEnabled ? mSampleTable[mCurrentSample] : 0;
}

void Channel::lengthStep() {
    if (!mContinuous) {
        if (mLengthCounter == 0) {
            mEnabled = false;
        } else {
            --mLengthCounter;
        }
    }
}

void Channel::reset() {
    mLengthCounter = mLength;
    mEnabled = true;
    mCurrentSample = Gbs::SAMPLE_MIN;
}

void Channel::setContinuousOutput(bool _continuous) {
    mContinuous = _continuous;
}

void Channel::setFrequency(uint16_t _frequency) {
    if (_frequency > Gbs::MAX_FREQUENCY) {
        _frequency = Gbs::MAX_FREQUENCY;
    }

    mFrequency = _frequency;
    mFreqCounterMax = calcFreqMax(mFrequency);
}

void Channel::setLength(uint8_t _length) {
    if (_length > Gbs::MAX_LENGTH) {
        _length = Gbs::MAX_LENGTH;
    }
    mLength = _length;
}

}