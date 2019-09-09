
#include "trackerboy/instrument.hpp"

#include <algorithm>

#define waveOffset(id) (id * WaveChannel::WAVE_RAMSIZE)

namespace trackerboy {

WaveTable::WaveTable()
{
}

uint8_t* WaveTable::getWave(uint8_t waveId) {
    size_t offset = waveOffset(waveId);
    if (offset >= mTable.size()) {
        return nullptr;
    } else {
        return mTable.data() + offset;
    }
}

void WaveTable::setWave(uint8_t waveId, uint8_t waveform[WaveChannel::WAVE_RAMSIZE]) {
    size_t offset = waveOffset(waveId);
    size_t tableSize = mTable.size();
    if (offset >= tableSize) {
        mTable.resize(offset + WaveChannel::WAVE_RAMSIZE);
        std::fill_n(mTable.begin() + tableSize, offset - tableSize, static_cast<uint8_t>(0));
    }

    std::copy_n(waveform, WaveChannel::WAVE_RAMSIZE, mTable.data() + offset);
}

std::vector<uint8_t>& WaveTable::table() {
    return mTable;
}

}