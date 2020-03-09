
#include "trackerboy/instrument/InstrumentRuntime.hpp"
#include "trackerboy/note.hpp"


namespace trackerboy {


InstrumentRuntime::InstrumentRuntime(ChType mTrackId) :
    mProgram(nullptr),
    mPc(0),
    mFc(1),
    mRunning(false),
    mTrackId(mTrackId)
{
}

void InstrumentRuntime::reset() {
    mPc = 0;
    mFc = 1;
    mRunning = mProgram != nullptr;
}

void InstrumentRuntime::setProgram(std::vector<Instruction> *_program) {
    mProgram = _program;
    reset();
}


void InstrumentRuntime::step(Synth &synth, WaveTable &wtable, uint8_t rowVol, uint16_t rowFreq) {

    if (mRunning && --mFc == 0) {

        if (mPc >= mProgram->size()) {
            mRunning = false;
            return;
        }

        Instruction inst = (*mProgram)[mPc++];
        mFc = inst.duration;

        HardwareFile &hf = synth.hardware();
        Envelope *env = nullptr;
        Osc *osc = nullptr;
        
        switch (mTrackId) {
            case ChType::ch1:
                env = &hf.env1;
                osc = &hf.osc1;
                // update sweep if set sweep flag is set
                if (inst.settings & Instruction::SETTINGS_SET_SWEEP) {
                    hf.sweep1.setRegister(inst.settings & 0x7F);
                }
                // forgive me for I must goto sin
                goto setduty;

            case ChType::ch2:
                env = &hf.env2;
                osc = &hf.osc2;

            setduty:
                if (inst.ctrl & Instruction::CTRL_SET_DUTY) {
                    static_cast<PulseOsc*>(osc)->setDuty(static_cast<Gbs::Duty>(inst.ctrl & Instruction::CTRL_DUTY));
                }
                break;

            case ChType::ch3:
                osc = &hf.osc3;

                if (inst.ctrl & Instruction::CTRL_SET_WAVE) {
                    // WAVE_SETLONG == WAVE_SET for now
                    Waveform *waveform = wtable[inst.envSettings];
                    if (waveform != nullptr) {
                        hf.osc3.setWaveform(*waveform);
                    }
                }

                // figure out wave volume later
                /*if (inst.ctrl & Instruction::CTRL_SET_WAVEVOL) {
                    cf.ch3.setOutputLevel(static_cast<Gbs::WaveVolume>(inst.settings & 0x3));
                }*/

                break;

            case ChType::ch4:
                env = &hf.env4;

                if (inst.ctrl & Instruction::CTRL_SET_NOISE) {
                    hf.gen4.setNoise(inst.settings);
                }

                break;
        }

        #undef executeDuty

        // execute settings for all instructions
        if (inst.ctrl & Instruction::CTRL_INIT) {
            // init sound, channel restarts output
            if (mTrackId == ChType::ch4) {
                hf.gen4.reset();
            } else {
                uint16_t freq;
                if (inst.note == NOTE_NONE) {
                    freq = rowFreq;
                } else {
                    freq = NOTE_FREQ_TABLE[inst.note];
                }
                osc->setFrequency(freq);
                osc->reset();
            }
        }

        if (inst.ctrl & Instruction::CTRL_PANNING) {
            bool leftEnable = inst.ctrl & Instruction::PANNING_LEFT;
            bool rightEnable = inst.ctrl & Instruction::PANNING_RIGHT;
            synth.setOutputEnable(mTrackId, Gbs::TERM_LEFT, leftEnable);
            synth.setOutputEnable(mTrackId, Gbs::TERM_RIGHT, rightEnable);
        }

        // envelope settings (all tracks except 3)

        if (mTrackId != ChType::ch3) {
            uint8_t envCtrl = inst.ctrl & Instruction::CTRL_SET_ENV;
            if (envCtrl != Instruction::ENV_NOSET) {
                uint8_t envsettings = inst.envSettings;
                if (envCtrl == Instruction::ENV_SETNOVOL) {
                    // replace volume (bits 4-7) with volume from row
                    envsettings = (rowVol << 4) | (envsettings & 0xF);
                }
                env->setRegister(envsettings);
            }
        }
    }
}

}