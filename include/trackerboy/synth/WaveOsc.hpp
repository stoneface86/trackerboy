#pragma once

#include "trackerboy/gbs.hpp"
#include "trackerboy/synth/Osc.hpp"
#include "trackerboy/synth/Waveform.hpp"


namespace trackerboy {


class WaveOsc : public Osc {

public:
    WaveOsc(float samplingRate);

    void setWaveform(Waveform &wave);


};

}
