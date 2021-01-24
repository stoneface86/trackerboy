
#include "trackerboy/data/Instrument.hpp"

#include "./checkedstream.hpp"

namespace trackerboy {


Instrument::Instrument() :
    DataItem(),
    mData{ 0 }
{
    // defaults
    mData.envelope = 0xF0;
    mData.panning = 0x11;
}

Instrument::Instrument(const Instrument &instrument) :
    DataItem(instrument),
    mData(instrument.mData)
{
}

Instrument::Data& Instrument::data() {
    return mData;
}

FormatError Instrument::serializeData(std::ostream &stream) noexcept {
    checkedWrite(stream, &mData, sizeof(Data));
    return FormatError::none;
}

FormatError Instrument::deserializeData(std::istream &stream) noexcept {
    checkedRead(stream, &mData, sizeof(Data));
    return FormatError::none;
}


}
