
#include "trackerboy/instrument/InstrumentRuntime.hpp"
#include "trackerboy/note.hpp"


namespace trackerboy {


InstrumentRuntime::InstrumentRuntime(ChType trackId) :
    program(nullptr),
    pc(0),
    fc(1),
    running(false),
    trackId(trackId)
{
}

void InstrumentRuntime::reset() {
    pc = 0;
    fc = 1;
    running = program != nullptr;
}

void InstrumentRuntime::setProgram(std::vector<Instruction> *_program) {
    program = _program;
    reset();
}


void InstrumentRuntime::step(Synth &synth, WaveTable &wtable, uint8_t rowVol, uint16_t rowFreq) {

    if (running && --fc == 0) {

        if (pc >= program->size()) {
            running = false;
            return;
        }

        Instruction inst = (*program)[pc++];
        fc = inst.duration;

        Channel *ch = nullptr;
        ChannelFile& cf = synth.getChannels();
       
        #define executeDuty(ch) do { \
            if (inst.ctrl & Instruction::CTRL_SET_DUTY) { \
                ch.setDuty(static_cast<Gbs::Duty>(inst.ctrl & Instruction::CTRL_DUTY)); \
            } \
        } while (false)
        
        switch (trackId) {
            case ChType::ch1:
                ch = &cf.ch1;
                // update sweep if set sweep flag is set
                if (inst.settings & Instruction::SETTINGS_SET_SWEEP) {
                    cf.ch1.setSweep(inst.settings & 0x7F);
                }
                executeDuty(cf.ch1);
                break;

            case ChType::ch2:
                ch = &synth.getChannels().ch2;
                
                executeDuty(cf.ch2);

                break;

            case ChType::ch3:
                ch = &cf.ch3;

                if (inst.ctrl & Instruction::CTRL_SET_WAVE) {
                    // WAVE_SETLONG == WAVE_SET for now
                    Waveform *waveform = wtable[inst.envSettings];
                    if (waveform != nullptr) {
                        cf.ch3.setWaveform(*waveform);
                    }
                }

                if (inst.ctrl & Instruction::CTRL_SET_WAVEVOL) {
                    cf.ch3.setOutputLevel(static_cast<Gbs::WaveVolume>(inst.settings & 0x3));
                }

                break;

            case ChType::ch4:
                ch = &cf.ch4;
                
                if (inst.ctrl & Instruction::CTRL_SET_NOISE) {
                    cf.ch4.setNoise(inst.settings);
                }

                break;
        }

        #undef executeDuty

        // execute settings for all instructions
        if (inst.ctrl & Instruction::CTRL_INIT) {
            // init sound, channel restarts output
            if (trackId != ChType::ch4) {
                uint16_t freq;
                if (inst.note == NOTE_NONE) {
                    freq = rowFreq;
                } else {
                    freq = NOTE_FREQ_TABLE[inst.note];
                }
                ch->setFrequency(freq);
            }
            ch->reset();
        }

        if (inst.ctrl & Instruction::CTRL_PANNING) {
            bool leftEnable = inst.ctrl & Instruction::PANNING_LEFT;
            bool rightEnable = inst.ctrl & Instruction::PANNING_RIGHT;
            auto &mixer = synth.getMixer();
            mixer.setEnable(trackId, Gbs::TERM_LEFT, leftEnable);
            mixer.setEnable(trackId, Gbs::TERM_RIGHT, rightEnable);
        }

        // envelope settings (all tracks except 3)

        if (trackId != ChType::ch3) {
            // downcast to ch to an EnvChannel (all channels except for ch3 subclass EnvChannel)
            EnvChannel* envCh = static_cast<EnvChannel*>(ch);
            uint8_t envCtrl = inst.ctrl & Instruction::CTRL_SET_ENV;
            if (envCtrl != Instruction::ENV_NOSET) {
                uint8_t envsettings = inst.envSettings;
                if (envCtrl == Instruction::ENV_SETNOVOL) {
                    // replace volume (bits 4-7) with volume from row
                    envsettings = (rowVol << 4) | (envsettings & 0xF);
                }
                envCh->setEnv(envsettings);
            }
        }
    }
}

}