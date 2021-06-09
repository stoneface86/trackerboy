
#include "trackerboy/data/Waveform.hpp"

#include <sstream>
#include <iomanip>

namespace trackerboy {


Waveform::Waveform() noexcept :
    DataItem(),
    mData{0}
{       
}

Waveform::Waveform(std::string const& hexstr) :
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


Waveform::Data& Waveform::data() noexcept {
    return mData;
}

Waveform::Data const& Waveform::data() const noexcept {
    return mData;
}

void Waveform::fromString(std::string const& hexstring) {
    for (size_t pos = 0, i = 0; pos < hexstring.size() && i != mData.size(); pos += 2, ++i) {
        std::string sub = hexstring.substr(pos, 2);
        uint8_t byte = (uint8_t)std::stoul(sub, nullptr, 16);
        if (sub.size() == 1) {
            byte = (byte << 4) | (mData[i] & 0xF);
        }

        mData[i] = byte;
    }
}

std::string Waveform::toString() const {
    std::stringstream ss;
    ss << std::hex;
    ss << std::uppercase;

    for (auto byte : mData) {
        ss << std::setw(2) << std::setfill('0') << (int)byte;
    }

    return ss.str();
}

uint8_t& Waveform::operator[](int index) {
    return mData[index];
}


}
