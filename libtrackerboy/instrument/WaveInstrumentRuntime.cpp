
#include "trackerboy/instrument.hpp"

namespace trackerboy {


WaveInstrumentRuntime::WaveInstrumentRuntime(WaveTable &waveTable) :
    mWaveTable(waveTable),
    InstrumentRuntime(TrackId::ch3)
{
}


void WaveInstrumentRuntime::execute(Synth &synth, Instruction inst) {
    
    auto &ch3 = synth.getChannels().ch3;

    if (inst.ctrl & Instruction::CTRL_SET_WAVE) {
        // WAVE_SETLONG == WAVE_SET for now
        uint8_t *waveform = mWaveTable.getWave(inst.envSettings);
        if (waveform != nullptr) {
            ch3.setWaveform(waveform);
        }
    }

    if (inst.ctrl & Instruction::CTRL_SET_WAVEVOL) {
        ch3.setOutputLevel(static_cast<WaveVolume>(inst.settings & 0x3));
    }
}


}
