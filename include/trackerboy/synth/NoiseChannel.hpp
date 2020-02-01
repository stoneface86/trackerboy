#pragma once

#include "EnvChannel.hpp"

#include "trackerboy/gbs.hpp"


namespace trackerboy {

class NoiseChannel : public EnvChannel {
    uint8_t mScf;
    Gbs::NoiseSteps mStepSelection;
    uint8_t mDrf;

    uint16_t mLfsr;
    unsigned mShiftCounter;
    unsigned mShiftCounterMax;

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
