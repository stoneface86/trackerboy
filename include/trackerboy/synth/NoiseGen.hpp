
#pragma once

#include "trackerboy/gbs.hpp"

#include "trackerboy/synth/Generator.hpp"


namespace trackerboy {

class NoiseGen : public Generator {

public:

    NoiseGen();

    void reset() override;

    //
    // channel retrigger. LFSR is re-initialized, counters are reset
    // and the period is reloaded from the set register
    //
    void restart() override;

    //
    // Step the generator for the given number of cycles, returning the
    // current output.
    //
    void step(uint32_t cycles);

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

    // width of the LFSR (15-bit or 7-bit)
    Gbs::NoiseSteps mStepSelection;
    // lfsr: linear feedback shift register
    uint16_t mLfsr;


};



}
