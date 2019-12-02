
#include "trackerboy/synth/Waveform.hpp"


namespace trackerboy {


Waveform::Waveform() :
    mData{0}
{       
}

uint8_t* Waveform::data() {
    return mData;
}

FormatError Waveform::deserialize(std::ifstream &stream) {
    return FormatError::none;
}

void Waveform::serialize(std::ofstream &stream) {
    stream.write(reinterpret_cast<const char*>(mData), Gbs::WAVE_RAMSIZE);
}


}