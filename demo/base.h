
#pragma once

#include "gbsynth.hpp"

namespace demo {

    enum Constants {
        NOERROR = 0,
        RUN_START = 0
    };

    //
    // One time initialization for this demo
    //
    int init(gbsynth::Synth &synth);

    //
    // Updates the synthesizer based on the given continuation value
    // The length of time in milleseconds this demo will run for is
    // to be set in the runtime reference. If 0 is set then the demo stops
    //
    int run(int cont, gbsynth::Synth &synth, long &runtime);

}