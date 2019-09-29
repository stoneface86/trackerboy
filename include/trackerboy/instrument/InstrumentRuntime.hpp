
#pragma once

#include <cstdint>
#include <vector>

#include "trackerboy/instrument/Instruction.hpp"
#include "trackerboy/instrument/WaveTable.hpp"
#include "trackerboy/synth/Synth.hpp"


namespace trackerboy {

class InstrumentRuntime {

    std::vector<Instruction>* program;
    uint8_t fc;
    uint8_t pc;
    bool running;
    const ChType trackId;

public:

    InstrumentRuntime(ChType trackId);

    void reset();

    void setProgram(std::vector<Instruction>* program);

    void step(Synth& synth, WaveTable& wtable, uint8_t rowVol = 0, uint16_t freq = 0);

};

}