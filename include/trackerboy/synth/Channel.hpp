#pragma once

#include <cstdint>

namespace trackerboy {

class Channel {
    uint8_t mLengthCounter;
    bool mContinuous;
    bool mEnabled;

protected:
    uint8_t mCurrentSample;
    uint8_t mLength;
    uint16_t mFrequency;
    const unsigned mFreqMultiplier;
    unsigned mFreqCounter;
    unsigned mFreqCounterMax;
    const int16_t *mSampleTable;

    Channel(bool ch3 = false);

public:

    virtual ~Channel() = default;

    void disable();
    uint8_t getCurrentSample();
    int16_t getCurrentVolume();
    void lengthStep();
    virtual void reset();
    void setContinuousOutput(bool continuous);
    void setFrequency(uint16_t frequency);
    void setLength(uint8_t length);
    virtual void step(unsigned cycles) = 0;
};



}