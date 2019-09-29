
#pragma once

#include <cstdint>
#include <vector>

#include "trackerboy/synth/WaveChannel.hpp"


namespace trackerboy {

class WaveTable {

public:
    WaveTable();

    uint8_t* getWave(uint8_t waveId);

    void setWave(uint8_t waveId, uint8_t waveform[WaveChannel::WAVE_RAMSIZE]);

    std::vector<uint8_t>& table();

private:

    std::vector<uint8_t> mTable;

};

}
