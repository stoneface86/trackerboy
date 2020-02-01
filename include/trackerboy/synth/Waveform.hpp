
#pragma once

#include <cstdint>
#include <fstream>

#include "trackerboy/gbs.hpp"
#include "trackerboy/fileformat.hpp"


namespace trackerboy {

class Waveform {

public:

    Waveform();

    uint8_t* data();

private:
    uint8_t mData[Gbs::WAVE_RAMSIZE];
};

}
