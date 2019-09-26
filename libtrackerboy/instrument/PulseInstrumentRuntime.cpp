
#include "trackerboy/instrument.hpp"

namespace trackerboy {


PulseInstrumentRuntime::PulseInstrumentRuntime(bool sweep) :
    InstrumentRuntime(sweep ? TrackId::ch1 : TrackId::ch2)
{
}

void PulseInstrumentRuntime::execute(Synth &synth, Instruction inst) {
    PulseChannel *ch;

    auto &cf = synth.getChannels();

    if (trackId == TrackId::ch1) {
        ch = &cf.ch1;
        // update sweep if set sweep flag is set
        if (inst.settings & Instruction::SETTINGS_SET_SWEEP) {
            cf.ch1.setSweep(inst.settings & 0x7F);
        }
    } else {
        ch = &cf.ch2;
    }

    if (inst.ctrl & Instruction::CTRL_SET_DUTY) {
        ch->setDuty(static_cast<Gbs::Duty>(inst.ctrl & Instruction::CTRL_DUTY));
    }
}



}


