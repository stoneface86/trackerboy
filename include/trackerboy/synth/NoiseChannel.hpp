#pragma once

#include "EnvChannel.hpp"

#include "trackerboy/gbs.hpp"


namespace trackerboy {

class NoiseChannel : public EnvChannel {
    uint8_t scf;
    Gbs::NoiseSteps stepSelection;
    uint8_t drf;

    uint16_t lfsr;
    unsigned shiftCounter;
    unsigned shiftCounterMax;

public:

    NoiseChannel();

    void reset() override;
    void setDrf(uint8_t drf);
    void setNoise(uint8_t noiseReg);
    void setScf(uint8_t scf);
    void setStepSelection(Gbs::NoiseSteps steps);
    void step(unsigned cycles) override;
};

}