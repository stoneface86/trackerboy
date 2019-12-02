
#pragma once

#include <fstream>
#include <vector>

#include "trackerboy/ChType.hpp"
#include "trackerboy/fileformat.hpp"
#include "trackerboy/instrument/Instruction.hpp"


namespace trackerboy {

class Instrument {

public:

    static constexpr uint8_t TABLE_CODE = 'I';

    Instrument();

    FormatError deserialize(std::ifstream &stream);

    std::vector<Instruction>& getProgram();

    void serialize(std::ofstream &stream);

private:

    // sequence of instructions, one instruction per frame
    std::vector<Instruction> mProgram;
};


}
