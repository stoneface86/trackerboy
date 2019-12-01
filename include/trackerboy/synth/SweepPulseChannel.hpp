#pragma once

#include "PulseChannel.hpp"

namespace trackerboy {

class SweepPulseChannel : public PulseChannel {
    Gbs::SweepMode mSweepMode;
    uint8_t mSweepTime;
    uint8_t mSweepShift;

    uint8_t mSweepCounter;

public:

    SweepPulseChannel();

    void reset() override;
    void setSweep(uint8_t sweepReg);
    void setSweepMode(Gbs::SweepMode mode);
    void setSweepShift(uint8_t n);
    void setSweepTime(uint8_t ts);
    void sweepStep();
};

}