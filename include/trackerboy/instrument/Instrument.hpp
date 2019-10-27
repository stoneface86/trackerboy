
#pragma once

#include <vector>

#include "trackerboy/ChType.hpp"
#include "trackerboy/instrument/Instruction.hpp"


namespace trackerboy {

class Instrument {

    uint8_t id;
    ChType trackId;
    // sequence of instructions, one instruction per frame
    std::vector<Instruction> program;

public:

    Instrument(uint8_t id, ChType track);

    uint8_t getId();
    ChType getTrackId();
    std::vector<Instruction>& getProgram();
};

}
