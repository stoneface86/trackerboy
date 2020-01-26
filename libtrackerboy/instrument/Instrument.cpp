
#include "trackerboy/instrument/Instrument.hpp"


namespace trackerboy {

Instrument::Instrument()
{
}

std::vector<Instruction>& Instrument::getProgram() {
    return mProgram;
}

}