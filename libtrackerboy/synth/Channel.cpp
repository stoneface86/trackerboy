
#include "trackerboy/synth/Channel.hpp"
#include "trackerboy/gbs.hpp"

#include "sampletable.hpp"

#define calcFreqMax(f) ((2048 - f) * mFreqMultiplier)


namespace trackerboy {


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
    mSampleTable(SAMPLE_TABLE)
{
}

void Channel::disable() {
    mEnabled = false;
}

uint8_t Channel::getCurrentSample() {
    return mEnabled ? mCurrentSample : Gbs::SAMPLE_MIN;
}

int16_t Channel::getCurrentVolume() {
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