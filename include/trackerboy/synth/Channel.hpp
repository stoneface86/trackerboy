#pragma once

#include <cstdint>

namespace trackerboy {

class Channel {
    uint8_t lengthCounter;
    bool continuous;
    bool enabled;

protected:
    uint8_t currentSample;
    uint8_t length;
    uint16_t frequency;
    const unsigned freqMultiplier;
    unsigned freqCounter;
    unsigned freqCounterMax;
    const int16_t *sampleTable;

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