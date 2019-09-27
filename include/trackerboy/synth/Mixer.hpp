#pragma once

#include "trackerboy/common.hpp"
#include "trackerboy/gbs.hpp"

#include <type_traits>


namespace trackerboy {

class Mixer {

public:

    Mixer();

    void getOutput(float in1, float in2, float in3, float in4, float &outLeft, float &outRight);
    void setEnable(Gbs::OutputFlags flags);
    void setEnable(ChType ch, Gbs::Terminal term, bool enabled);
    void setTerminalEnable(Gbs::Terminal term, bool enabled);
    void setTerminalVolume(Gbs::Terminal term, uint8_t volume);

private:
    bool s01enable, s02enable;
    uint8_t s01vol, s02vol;
    std::underlying_type<Gbs::OutputFlags>::type outputStat;

};

}