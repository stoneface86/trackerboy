#pragma once

#include "SweepPulseChannel.hpp"
#include "PulseChannel.hpp"
#include "WaveChannel.hpp"
#include "NoiseChannel.hpp"

namespace trackerboy {

struct ChannelFile {
    SweepPulseChannel ch1;
    PulseChannel ch2;
    WaveChannel ch3;
    NoiseChannel ch4;
};

}
