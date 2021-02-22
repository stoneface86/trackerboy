
#include "trackerboy/data/Waveform.hpp"

#include "./checkedstream.hpp"

namespace trackerboy {


Waveform::Waveform() noexcept :
    DataItem(),
    mData{0}
{       
}

Waveform::Waveform(std::string &hexstr) :
    mData{ 0 }
{
    fromString(hexstr);
}

Waveform::Waveform(const Waveform &wave) :
    DataItem(wave),
    mData{ 0 }
{
    std::copy(wave.mData.begin(), wave.mData.end(), mData.begin());
}


uint8_t* Waveform::data() noexcept {
    return mData.data();
}

void Waveform::fromString(std::string hexstring) {
    for (size_t pos = 0, i = 0; pos < hexstring.size() && i != Gbs::WAVE_RAMSIZE; pos += 2, ++i) {
        std::string sub = hexstring.substr(pos, 2);
        uint8_t byte = (uint8_t)std::stoul(sub, nullptr, 16);
        if (sub.size() == 1) {
            byte = (byte << 4) | (mData[i] & 0xF);
        }

        mData[i] = byte;
    }
}

uint8_t& Waveform::operator[](int index) {
    return mData[index];
}

FormatError Waveform::serializeData(std::ostream &stream) noexcept {
    checkedWrite(stream, mData.data(), Gbs::WAVE_RAMSIZE);
    return FormatError::none;
}

FormatError Waveform::deserializeData(std::istream &stream) noexcept {
    checkedRead(stream, mData.data(), Gbs::WAVE_RAMSIZE);
    return FormatError::none;
}


}
