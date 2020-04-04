
#include "trackerboy/instrument/InstrumentRuntime.hpp"
#include "trackerboy/note.hpp"

//
// Notes on Instrument Streams
//
// Instrument streams are a sequence of commands that are used to set
// APU registers on a per frame basis. The size of a command is variable,
// but all have a minimum size of 1 byte. The first byte in a command is
// control byte (ctrl). If the control byte is not used, then the byte
// is actually an xcontrol byte (xctrl, eXtended control). See the following
// for the possible command forms. #3 is a nop, so the register will remain
// unchanged
//
// Command forms:
// 1. <ctrl> [<xctrl> [tune] [fine]] [aux1] [aux2]
// 2. <xctrl> [tune] [fine]
// 3. 0x00
//
// All fields in the above forms are 1 byte. A command size ranges from 1-6 bytes
// See Instrument::Flags for the bit fields of the ctrl and xctrl bytes
//
// Stepping the runtime for an instrument runs one command. 1 step == 1 frame
// The runtime goes through every command in the stream and then stops, no
// further writes to the registers are made when the stream is finished.
//
//
// Example: pulsar (CH1)
// this instrument sets CH1's envelope to D0, sweep to 00, and duty to 50% on the
// first frame then it changes the duty for the next three frames
//
// 00 | I D\0 0-0 2 .. .. ..
// 01 | . ... ... 1 .. .. ..
// 02 | . ... ... 0 .. .. ..
// 03 | . ... ... 1 .. .. ..
//
// The stream:
// Frame 0: 0x5E 0x00 0xD0      => retrigger, sweep = 0x00, envelope = 0xD0
// Frame 1: 0x05                => duty = 0x01
// Frame 2: 0x04                => duty = 0x00
// Frame 3: 0x05                => duty = 0x01
//
// So the stream would be [ 0x5E, 0x00, 0xD0, 0x05, 0x04, 0x05 ]
//
// Note that frame 0 has both aux bytes enabled. For CH1, aux1 sets the sweep
// register, and aux2 sets the envelope register. The remaining frames only have
// the ctrl byte, as they just set the duty (CH1/CH2 use ctrl's setting field
// for duty setting).
//
// In general, "simple" instruments have 1 command per stream. These
// instruments just setup the registers on use.
//
// Complex instruments can have ADSR envelopes by setting the envelope register
// at different points in time (frames). The possibilities are endless
//

//
// Other ideas for instruments:
// 1. Sequences: this is how famitracker does instruments, where a parameter (volume, pitch)
//    changes from a sequence of values, 1 change per frame. This is doable, but complicated,
//    as each runtime would need counters for each sequence. (might need a SequenceRuntime perhaps)
//    I'm also not sure if it is feasible for implementing on the hardware, memory-wise.
//    Volume sequences would definitely not be implemented, as software volume control is
//    finicky on the gameboy. Compared to commands, sequences are more software focused, which
//    will add more complexity to the driver.
//
// 2. Loop/release points. This is an addition to the current implementation. A loop point
//    will cause the runtime to loop back when it reaches the end of a stream or a release point.
//    The runtime will jump to a release point when a special note, note release, is triggered.
//

namespace trackerboy {

InstrumentRuntimeBase::InstrumentRuntimeBase() :
    mProgram(nullptr),
    mPc(0),
    mPitchOffset(0),
    mFinePitchOffset(0),
    mRunning(false)
{
}


void InstrumentRuntimeBase::reset() {
    mPc = 0;
    mPitchOffset = 0;
    mFinePitchOffset = 0;
    mRunning = mProgram != nullptr;
}


void InstrumentRuntimeBase::setProgram(std::vector<uint8_t> *program) {
    mProgram = program;
    reset();
}



template <ChType ch>
InstrumentRuntime<ch>::InstrumentRuntime() :
    InstrumentRuntimeBase()
{
}

template <ChType trackId>
void InstrumentRuntime<trackId>::step(Synth &synth, WaveTable &wtable, uint16_t rowFreq) {
    if constexpr (trackId != ChType::ch3) {
        // only CH3 needs the wave table
        (void)wtable;
    }

    if constexpr (trackId == ChType::ch4) {
        // CH4 doesn't use note frequency
        (void)rowFreq;
    }

    if (mRunning) {

        if (mPc >= mProgram->size()) {
            mRunning = false;
            return;
        }

        auto &prgm = *mProgram;
        auto &hf = synth.hardware();

        bool retrigger = false;
        bool ctrlExists = true;

        // read the ctrl byte from the program
        uint8_t ctrl = prgm[mPc++];
        uint8_t xctrl = 0;
        if (ctrl & Instrument::CTRL_FLAG_ISXCTRL) {
            // ctrl is actually an xctrl
            xctrl = ctrl;
            ctrlExists = false;
        } else if (ctrl & Instrument::CTRL_FLAG_XCTRL_EN) {
            // next byte is xctrl
            xctrl = prgm[mPc++];
        }

        if (xctrl) {
            if (xctrl & Instrument::XCTRL_FLAG_TUNING_EN) {
                mPitchOffset = static_cast<int8_t>(prgm[mPc++]);
            }

            if (xctrl & Instrument::XCTRL_FLAG_FINE_EN) {
                mFinePitchOffset = static_cast<int8_t>(prgm[mPc++]);
            }

            if (xctrl & Instrument::XCTRL_FLAG_PANNING_EN) {
                uint8_t panning = xctrl & Instrument::XCTRL_FLAG_PANNING;
                synth.setOutputEnable(trackId, Gbs::TERM_LEFT, panning & 1);
                synth.setOutputEnable(trackId, Gbs::TERM_RIGHT, panning >> 1);
            }
        }

        if (ctrlExists) {
            // parse the rest of the ctrl byte
            retrigger = !!(ctrl & Instrument::CTRL_FLAG_RESTART);
            
            if (ctrl & Instrument::CTRL_FLAG_AUX1_EN) {
                uint8_t aux1 = prgm[mPc++];
                switch (trackId) {
                    case ChType::ch1:
                        // aux1 is set sweep register
                        synth.writeRegister(Gbs::REG_NR10, aux1);
                        break;
                    case ChType::ch2:
                        // aux1 is unused
                        break;
                    case ChType::ch3:
                        // aux1 is unused
                        break;
                    case ChType::ch4:
                        // aux1 is set noise register
                        synth.writeRegister(Gbs::REG_NR43, aux1);
                        break;
                }
            }

            if (ctrl & Instrument::CTRL_FLAG_AUX2_EN) {
                uint8_t aux2 = prgm[mPc++];
                if constexpr (trackId == ChType::ch3) {
                    // aux2 is the waveform index
                    auto *waveform = wtable[aux2];
                    if (waveform != nullptr) {
                        hf.gen3.copyWave(*waveform);
                    }
                } else {
                    // aux2 is the set envelope register
                    constexpr uint16_t ENV_REGS[4] = {
                        Gbs::REG_NR12, Gbs::REG_NR22, 0, Gbs::REG_NR42
                    };
                    synth.writeRegister(ENV_REGS[static_cast<int>(trackId)], aux2);
                }
            }

            if (ctrl & Instrument::CTRL_FLAG_SETTINGS_EN) {
                uint8_t settings = ctrl & Instrument::CTRL_FLAG_SETTINGS;
                switch (trackId) {
                    case ChType::ch1:
                        hf.gen1.setDuty(static_cast<Gbs::Duty>(settings));
                        break;
                    case ChType::ch2:
                        hf.gen2.setDuty(static_cast<Gbs::Duty>(settings));
                        break;
                    case ChType::ch3:
                        hf.gen3.setVolume(static_cast<Gbs::WaveVolume>(settings));
                        break;
                    case ChType::ch4:
                        break;
                }
            }
        }


        if constexpr (trackId != ChType::ch4) {
            // pitch adjustment
            int16_t ifreq = rowFreq + mPitchOffset + (static_cast<int16_t>(mFinePitchOffset) << 4);
            uint16_t freq;
            if (ifreq < 0) {
                freq = 0;
            } else if (ifreq > Gbs::MAX_FREQUENCY) {
                freq = Gbs::MAX_FREQUENCY;
            } else {
                freq = static_cast<uint16_t>(ifreq);
            }
            // set the frequency
            switch (trackId) {
                case ChType::ch1:
                    hf.gen1.setFrequency(freq);
                    break;
                case ChType::ch2:
                    hf.gen2.setFrequency(freq);
                    break;
                case ChType::ch3:
                    hf.gen3.setFrequency(freq);
                    break;
                case ChType::ch4:
                    break;
            }
        }

        if (retrigger) {
            synth.restart(trackId);
        }
    }
}


template class InstrumentRuntime<ChType::ch1>;
template class InstrumentRuntime<ChType::ch2>;
template class InstrumentRuntime<ChType::ch3>;
template class InstrumentRuntime<ChType::ch4>;


}
