
#include "trackerboy/instrument.hpp"

namespace trackerboy {


NoiseInstrumentRuntime::NoiseInstrumentRuntime() :
    InstrumentRuntime(TrackId::ch4)
{
}


void NoiseInstrumentRuntime::execute(Synth &synth, Instruction inst) {

    if (inst.ctrl & Instruction::CTRL_SET_NOISE) {
        synth.getChannels().ch4.setNoise(inst.settings);
    }


}


}
