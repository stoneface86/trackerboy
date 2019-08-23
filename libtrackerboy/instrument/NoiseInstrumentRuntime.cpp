
#include "trackerboy/instrument.hpp"

namespace trackerboy {


NoiseInstrumentRuntime::NoiseInstrumentRuntime() :
    InstrumentRuntime(TrackId::ch4)
{
}


void NoiseInstrumentRuntime::execute(Synth &synth, Instruction inst) {
    // TODO: implement noise runtime
}


}
