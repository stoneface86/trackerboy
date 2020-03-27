
#pragma once

#include "trackerboy/gbs.hpp"


namespace trackerboy {

class NoiseGen {

public:

    NoiseGen();

    //
    // channel retrigger. LFSR is re-initialized, counters are reset
    // and the period is reloaded from the set register
    //
    void restart();

    //
    // Step the generator for the given number of cycles, returning the
    // current output.
    //
    uint8_t step(unsigned cycles);

    //
    // Write the given value to this generator's register, NR43
    //
    void writeRegister(uint8_t reg);

    //
    // Returns the contents of this generator's register
    //
    uint8_t readRegister();

private:

    // NR43 register contents
    uint8_t mRegister;

    // scf: Shift clock frequency
    uint8_t mScf;
    // width of the LFSR (15-bit or 7-bit)
    Gbs::NoiseSteps mStepSelection;
    // drf: Dividing ratio frequency
    uint8_t mDrf;
    // lfsr: linear feedback shift register
    uint16_t mLfsr;

    // frequency counter
    unsigned mShiftCounter;
    unsigned mShiftCounterMax;

};



}
