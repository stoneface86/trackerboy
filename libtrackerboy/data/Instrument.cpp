
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



}
