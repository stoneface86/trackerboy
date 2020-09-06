
#pragma once

#include <array>
#include <cstdint>
#include <fstream>

#include "trackerboy/data/DataItem.hpp"
#include "trackerboy/gbs.hpp"
#include "trackerboy/fileformat.hpp"


namespace trackerboy {

class Waveform : public DataItem {

public:

    Waveform() noexcept;

    Waveform(std::string &hexstring);

    uint8_t* data() noexcept;

    // convenience method, sets the waveform data from a string of hex nibbles
    void fromString(std::string hexstring);

    uint8_t& operator[](int index);

protected:
    virtual bool serializeData(std::ostream &stream) noexcept override;
    virtual bool deserializeData(std::istream &stream) noexcept override;


private:
    //uint8_t mData[Gbs::WAVE_RAMSIZE];
    std::array<uint8_t, Gbs::WAVE_RAMSIZE> mData;
};

}
