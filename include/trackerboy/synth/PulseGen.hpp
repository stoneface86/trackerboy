#pragma once

#include <cstdint>

#include "trackerboy/gbs.hpp"
#include "trackerboy/synth/Generator.hpp"


namespace trackerboy {

class PulseGen : public Generator {

public:

    PulseGen();

    //
    // Restart the generator, counters are reset to 0
    //
    void restart() override;

    //
    // Step the generator for the given number of cycles, returning the
    // current output (1 for output on, 0 for off)
    //
    void step(uint32_t cycles);

    //
    // Set the duty of the pulse. Does not require restart.
    //
    void setDuty(Gbs::Duty duty);

    //
    // Set the frequency of the output waveform. Does not require restart. If a sweep
    // is being applied to this generator, any changes will be lost on next sweep
    // trigger. (ie, changing this frequency does not modify the sweep's shadow
    // frequency).
    //
    void setFrequency(uint16_t frequency);

private:

    uint16_t mFrequency;
    Gbs::Duty mDuty;
    
    unsigned mDutyCounter;


};



}
