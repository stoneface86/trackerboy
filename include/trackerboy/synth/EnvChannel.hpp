#pragma once

#include "Channel.hpp"
#include "trackerboy/gbs.hpp"

namespace trackerboy {

class EnvChannel : public Channel {
    uint8_t mEnvCounter;

protected:
    uint8_t mEnvelope;
    Gbs::EnvMode mEnvMode;
    uint8_t mEnvLength;

    EnvChannel();

public:

    virtual ~EnvChannel() = default;

    void envStep();
    void reset() override;
    void setEnv(uint8_t envReg);
    void setEnvLength(uint8_t length);
    void setEnvMode(Gbs::EnvMode mode);
    void setEnvStep(uint8_t step);
};

}