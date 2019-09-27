#pragma once

#include "EnvChannel.hpp"
#include "trackerboy/gbs.hpp"

namespace trackerboy {

class PulseChannel : public EnvChannel {
    Gbs::Duty duty;
    unsigned dutyCounter;

public:

    PulseChannel();

    virtual void reset() override;
    void setDuty(Gbs::Duty duty);
    void step(unsigned cycles) override;

};

}