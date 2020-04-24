
#include "trackerboy/synth/Waveform.hpp"


namespace trackerboy {


Waveform::Waveform() noexcept :
    mData{0}
{       
}

Waveform::Waveform(std::string &hexstr) :
    mData{ 0 }
{
    fromString(hexstr);
}


uint8_t* Waveform::data() noexcept {
    return mData.data();
}

void Waveform::fromString(std::string hexstring) {
    for (size_t pos = 0, i = 0; pos < hexstring.size() && i != Gbs::WAVE_RAMSIZE; pos += 2, ++i) {
        std::string sub = hexstring.substr(pos, 2);
        uint8_t byte = std::stoul(sub, nullptr, 16);
        if (sub.size() == 1) {
            byte = (byte << 4) | (mData[i] & 0xF);
        }

        mData[i] = byte;
    }
}

uint8_t& Waveform::operator[](int index) {
    return mData[index];
}


}
