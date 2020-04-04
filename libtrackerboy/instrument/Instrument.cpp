
#include "trackerboy/instrument/Instrument.hpp"


namespace trackerboy {

Instrument::Instrument()
{
}

std::vector<uint8_t>& Instrument::getProgram() {
    return mStream;
}

}
