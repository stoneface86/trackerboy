
#pragma once

#include "trackerboy/gbs.hpp"
#include "trackerboy/synth/Osc.hpp"

#include <cstdint>


namespace trackerboy {


class Sweep {

public:

    Sweep(Osc &osc);

    void reset();

    void setRegister(uint8_t reg);

    void trigger();

private:

    Osc &mOsc;

    Gbs::SweepMode mSweepMode;
    uint8_t mSweepTime;
    uint8_t mSweepShift;

    uint8_t mSweepCounter;

};


}