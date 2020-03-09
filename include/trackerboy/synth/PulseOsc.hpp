
#pragma once

#include "trackerboy/synth/Osc.hpp"
#include "trackerboy/gbs.hpp"


namespace trackerboy {


class PulseOsc : public Osc {

public:
    PulseOsc(float samplingRate);

    void setDuty(Gbs::Duty duty);

private:
    void setDeltaBuf();

    Gbs::Duty mDuty;

};



}
