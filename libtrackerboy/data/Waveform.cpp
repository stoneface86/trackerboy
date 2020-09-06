
#include "trackerboy/data/Waveform.hpp"


namespace trackerboy {


Waveform::Waveform() noexcept :
    mData{0},
    DataItem()
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

bool Waveform::serializeData(std::ofstream &stream) noexcept {
    stream.write(reinterpret_cast<const char *>(mData.data()), Gbs::WAVE_RAMSIZE);
    return stream.good();
}

bool Waveform::deserializeData(std::ifstream &stream) noexcept {
    stream.read(reinterpret_cast<char*>(mData.data()), Gbs::WAVE_RAMSIZE);
    return stream.good();
}


}
