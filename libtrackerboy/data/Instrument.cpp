
#include "trackerboy/data/Instrument.hpp"


namespace trackerboy {


Instrument::Instrument() :
    mData{ 0 },
    DataItem()
{
}

Instrument::Data& Instrument::data() {
    return mData;
}

bool Instrument::serializeData(std::ofstream &stream) noexcept {
    stream.write(reinterpret_cast<const char *>(&mData), sizeof(Data));
    return stream.good();
}

bool Instrument::deserializeData(std::ifstream &stream) noexcept {
    stream.read(reinterpret_cast<char*>(&mData), sizeof(Data));
    return stream.good();
}


}
