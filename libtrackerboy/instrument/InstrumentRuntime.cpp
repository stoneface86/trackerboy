
#include "trackerboy/instrument.hpp"


namespace trackerboy {


InstrumentRuntime::InstrumentRuntime(TrackId trackId) :
    program(nullptr),
    pc(0),
    fc(1),
    trackId(trackId)
{
}

bool InstrumentRuntime::isFinished() {
    return program != nullptr && pc >= program->size() && fc == 1;
}

void InstrumentRuntime::reset() {
    pc = 0;
    fc = 1;
}

void InstrumentRuntime::setProgram(std::vector<Instruction> *_program) {
    program = _program;
    reset();
}


void InstrumentRuntime::step(Synth &synth) {

    if (program == nullptr) {
        return;
    }

    if (--fc == 0) {

        if (pc >= program->size()) {
            return;
        }

        Instruction inst = (*program)[pc++];
        fc = inst.duration;
        execute(synth, inst);

        Channel *ch = nullptr;
        EnvChannel *envCh = nullptr;
        // TODO: might need to extract this later (ie channelFromTrackId)
        switch (trackId) {
            case TrackId::ch1:
                envCh = &synth.getChannels().ch1;
                ch = envCh;
                break;
            case TrackId::ch2:
                envCh = &synth.getChannels().ch2;
                ch = envCh;
                break;
            case TrackId::ch3:
                ch = &synth.getChannels().ch3;
                break;
            case TrackId::ch4:
                envCh = &synth.getChannels().ch4;
                ch = envCh;
                break;
        }

        // execute settings for all instructions
        if (inst.ctrl & Instruction::CTRL_INIT) {
            // init sound, channel restarts output
            ch->reset();
        }

        if (inst.ctrl & Instruction::CTRL_PANNING) {
            bool leftEnable = inst.ctrl & Instruction::PANNING_LEFT;
            bool rightEnable = inst.ctrl & Instruction::PANNING_RIGHT;
            auto &mixer = synth.getMixer();
            mixer.setEnable(static_cast<ChType>(trackId), Mixer::term_left, leftEnable);
            mixer.setEnable(static_cast<ChType>(trackId), Mixer::term_right, rightEnable);
        }

        // envelope settings (all tracks except 3)

        if (envCh != nullptr) {
            uint8_t envCtrl = inst.ctrl & Instruction::CTRL_SET_ENV;
            if (envCtrl != Instruction::ENV_NOSET) {
                uint8_t envsettings = inst.envSettings;
                if (envCtrl == Instruction::ENV_SETNOVOL) {
                    // TODO: get the volume from the current note
                    // replace volume (bits 4-7) with volume from row
                }
                envCh->setEnv(envsettings);
            }
        }
    }
}

}