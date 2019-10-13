#pragma once

#include "Channel.hpp"
#include "trackerboy/gbs.hpp"

namespace trackerboy {

class EnvChannel : public Channel {
    uint8_t envCounter;

protected:
    uint8_t envelope;
    Gbs::EnvMode envMode;
    uint8_t envLength;

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