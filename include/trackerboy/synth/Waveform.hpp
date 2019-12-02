
#pragma once

#include <cstdint>
#include <fstream>

#include "trackerboy/gbs.hpp"
#include "trackerboy/fileformat.hpp"


namespace trackerboy {

class Waveform {

public:

    static constexpr uint8_t TABLE_CODE = 'W';

    Waveform();

    uint8_t* data();

    FormatError deserialize(std::ifstream &stream);

    void serialize(std::ofstream &stream);

private:
    uint8_t mData[Gbs::WAVE_RAMSIZE];
};

}