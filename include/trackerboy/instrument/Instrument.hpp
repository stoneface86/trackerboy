
#pragma once

#include <vector>

#include "trackerboy/ChType.hpp"
#include "trackerboy/instrument/Instruction.hpp"


namespace trackerboy {

class Instrument {

public:

    Instrument();

    std::vector<Instruction>& getProgram();

private:

    // sequence of instructions, one instruction per frame
    std::vector<Instruction> mProgram;
};


}
