
#pragma once


#include "trackerboy/synth/Generator.hpp"
#include "trackerboy/synth/Waveform.hpp"


namespace trackerboy {

class WaveGen : public Generator {

public:

    WaveGen();

    void copyWave(Waveform &wave);

    uint16_t frequency();

    void reset() override;

    void restart() override;

    void setFrequency(uint16_t frequency);

    void setVolume(Gbs::WaveVolume volume);

    void step(uint32_t cycles);

    Gbs::WaveVolume volume();

private:

    uint16_t mFrequency;
    Gbs::WaveVolume mVolume;
    uint8_t mWaveIndex;
    uint8_t mWaveram[Gbs::WAVE_RAMSIZE];


};

}
