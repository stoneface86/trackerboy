
#pragma once

#include <cstdint>
#include <fstream>

#include "trackerboy/gbs.hpp"


namespace trackerboy {

class Waveform {

public:

    static constexpr uint8_t TABLE_CODE = 'W';

    Waveform();

    uint8_t* data();

    void serialize(std::ofstream &stream);

private:
    uint8_t mData[Gbs::WAVE_RAMSIZE];
};

}