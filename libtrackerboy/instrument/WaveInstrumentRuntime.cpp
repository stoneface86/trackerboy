
#include "trackerboy/instrument.hpp"

namespace trackerboy {


WaveInstrumentRuntime::WaveInstrumentRuntime() :
    InstrumentRuntime(TrackId::ch3)
{
}


void WaveInstrumentRuntime::execute(Synth &synth, Instruction inst) {
    // TODO: implement wave runtime
}


}
