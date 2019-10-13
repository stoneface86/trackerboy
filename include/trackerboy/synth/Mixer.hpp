#pragma once

#include "trackerboy/common.hpp"
#include "trackerboy/gbs.hpp"

#include <cstdint>
#include <type_traits>


namespace trackerboy {

class Mixer {

public:

    Mixer();

    void getOutput(int16_t in1, int16_t in2, int16_t in3, int16_t in4, int16_t &outLeft, int16_t &outRight);
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