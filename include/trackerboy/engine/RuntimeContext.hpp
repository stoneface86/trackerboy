
#pragma once

#include "trackerboy/synth/Synth.hpp"
#include "trackerboy/Table.hpp"

namespace trackerboy {

//
// The RuntimeContext struct is a utility struct that contains references for a Synth
// and data tables.
//
struct RuntimeContext {

    RuntimeContext(Synth &synth, InstrumentTable &instTable, WaveTable &waveTable);

    Synth &synth;
    InstrumentTable &instTable;
    WaveTable &waveTable;


};


}
