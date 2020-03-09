
#pragma once

#include "trackerboy/synth/Envelope.hpp"
#include "trackerboy/synth/Sweep.hpp"
#include "trackerboy/synth/PulseOsc.hpp"
#include "trackerboy/synth/WaveOsc.hpp"
#include "trackerboy/synth/NoiseGen.hpp"

namespace trackerboy {

//
// POD struct for the individual hardware components of the synthesizer.
// A number in the field name indicates the channel it belongs to.
//
struct HardwareFile {

    Envelope env1, env2, env4;
    Sweep sweep1;
    PulseOsc osc1, osc2;
    WaveOsc osc3;
    NoiseGen gen4;

    HardwareFile(float samplingRate) :
        env1(),
        env2(),
        env4(),
        sweep1(osc1),
        osc1(samplingRate),
        osc2(samplingRate),
        osc3(samplingRate),
        gen4()
    {
    }

};



}