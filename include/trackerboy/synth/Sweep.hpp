
#pragma once

#include "trackerboy/gbs.hpp"
#include "trackerboy/synth/PulseGen.hpp"

#include <cstdint>


namespace trackerboy {


class Sweep {

public:

    Sweep(PulseGen &gen);

    uint8_t readRegister();

    void restart();

    void writeRegister(uint8_t reg);

    void trigger();

private:

    PulseGen &mGen;

    Gbs::SweepMode mSweepMode;
    uint8_t mSweepTime;
    uint8_t mSweepShift;

    uint8_t mSweepCounter;

    // Sweep register, NR10
    // Bits 0-2: Shift amount
    // Bit    3: Sweep mode (1 = subtraction)
    // Bits 4-6: Period
    uint8_t mRegister;

    // shadow register, CH1's frequency gets copied here on reset (initalization)
    int16_t mShadow;

};


}
