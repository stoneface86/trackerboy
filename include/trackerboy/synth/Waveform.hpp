
#pragma once

#include <cstdint>
#include <fstream>

#include "trackerboy/gbs.hpp"
#include "trackerboy/fileformat.hpp"


namespace trackerboy {

class Waveform {

public:

    Waveform() noexcept;

    uint8_t* data() noexcept;

private:
    uint8_t mData[Gbs::WAVE_RAMSIZE];
};

}
