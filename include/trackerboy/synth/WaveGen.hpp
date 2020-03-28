
#pragma once


#include "trackerboy/synth/Generator.hpp"
#include "trackerboy/synth/Waveform.hpp"


namespace trackerboy {

class WaveGen : public Generator {

public:

    WaveGen();

    void copyWave(Waveform &wave);

    void restart() override;

    void setFrequency(uint16_t frequency);

    void setVolume(Gbs::WaveVolume volume);

    void step(uint32_t cycles);

private:

    Gbs::WaveVolume mVolume;
    uint8_t mWaveIndex;
    uint8_t mWaveram[Gbs::WAVE_RAMSIZE];


};

}
