#pragma once

#include "PulseChannel.hpp"

namespace trackerboy {

class SweepPulseChannel : public PulseChannel {
    Gbs::SweepMode sweepMode;
    uint8_t sweepTime;
    uint8_t sweepShift;

    uint8_t sweepCounter;

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