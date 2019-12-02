
#include "trackerboy/instrument/Instrument.hpp"


namespace trackerboy {

Instrument::Instrument()
{
}

FormatError Instrument::deserialize(std::ifstream &stream) {
    return FormatError::none;
}

std::vector<Instruction>& Instrument::getProgram() {
    return mProgram;
}

void Instrument::serialize(std::ofstream &stream) {
    // write the size (1 byte)
    uint8_t size = static_cast<uint8_t>(mProgram.size());
    stream.write(reinterpret_cast<const char *>(&size), sizeof(uint8_t));
    // write the program (size * sizeof(Instruction))
    for (auto iter = mProgram.cbegin(); iter != mProgram.cend(); ++iter) {
        stream.write(reinterpret_cast<const char *>(&(*iter)), sizeof(Instruction));
    }
}

}