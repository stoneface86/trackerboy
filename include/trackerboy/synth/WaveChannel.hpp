#pragma once

#include "Channel.hpp"

#include "trackerboy/gbs.hpp"


namespace trackerboy {

class WaveChannel : public Channel {

public:
    static constexpr size_t WAVE_SIZE = 32;
    static constexpr size_t WAVE_RAMSIZE = 16;

    WaveChannel();

    void reset() override;
    void setOutputLevel(Gbs::WaveVolume level);
    void setWaveform(const uint8_t buf[WAVE_RAMSIZE]);
    void step(unsigned cycles) override;

private:
    Gbs::WaveVolume outputLevel;
    uint8_t wavedata[WAVE_RAMSIZE];
    unsigned waveIndex;

};

}