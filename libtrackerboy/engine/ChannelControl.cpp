
#include "trackerboy/engine/ChannelControl.hpp"
#include "trackerboy/engine/FrequencyControl.hpp"
#include "trackerboy/note.hpp"

#include "internal/enumutils.hpp"

namespace trackerboy {

template <ChType ch>
void ChannelControl<ch>::update(
    IApu &apu,
    WaveformList const& waveList,
    ChannelState const& lastState,
    ChannelState const& state
) noexcept {

    if constexpr (ch != ChType::ch3) {
        (void)waveList; // waveList is only needed for CH3
    }

    // retrigger the channel when:
    //  1. a note is triggered on an envelope channel (state.playing transitions from false -> true)
    //  2. the envelope is changed
    //  3. the override is set (happens a new note plays or instrument is reloaded)
    bool retrigger = state.retrigger;

    // starting register address for the channel (each channel has 5 registers)
    // Offset   CH1         CH2         CH3         CH4
    //  +0      sweep       N/A         DAC         N/A
    //  +1      duty/LC     duty/LC     LC          LC
    //  +2      envelope    envelope    volume      envelope
    //  +3                 freq LSB                 noise
    //  +4            freq MSB / retrigger          retrigger
    constexpr uint8_t REGS_START = gbapu::Apu::REG_NR10 + (+ch * GB_CHANNEL_REGS);


    // there are two ways to silence a channel:
    //  1. disable the DAC
    //  2. mute via NR51
    // #1 is easier to do since it doesn't require any bitwiddling, but it does destroy
    // the contents of the envelope register (we have to restore it on note trigger). CH3
    // doesn't require any restoring since we just write to NR30
    // #2 only requires us to restore panning on note trigger and the process for
    // disabling/enabling each channel is the same. Downside is we have read the NR51 register,
    // modify it, then write it back. (#1 just requires a write)
    //
    // We'll go with #2 since it's the same process for each channel and updating panning
    // is much easier to deal with than the envelope

    bool writePanning = lastState.panning != state.panning;
    bool writeEnvelope = lastState.envelope != state.envelope;

    if (lastState.playing != state.playing) {
        if (state.playing) {
            // note trigger
            // reload panning + envelope register
            writePanning = true;

            if constexpr (ch != ChType::ch3) {
                writeEnvelope = true; // reload envelope register on note trigger
            }

        } else {
            // note cut
            // set panning to mute
            uint8_t nr51 = apu.readRegister(gbapu::Apu::REG_NR51);
            nr51 &= ~(0x11 << +ch);
            apu.writeRegister(gbapu::Apu::REG_NR51, nr51);
        }
    }


    if (writeEnvelope) {

        // changing the envelope requires a retrigger for all channels
        // write the envelope
        if constexpr (ch == ChType::ch3) {
            auto waveform = waveList[state.envelope];
            // TODO: we don't need a shared_ptr here since the wave data is copied
            // might be worth adding a getRaw method to DataList that gets a raw pointer
            if (waveform != nullptr) {
                // DAC OFF
                apu.writeRegister(gbapu::Apu::REG_NR30, 0x00);

                // copy wave
                auto &data = waveform->data();
                for (size_t i = 0; i != data.size(); ++i) {
                    apu.writeRegister((uint8_t)(gbapu::Apu::REG_WAVERAM + i), data[i]);
                }

                // DAC ON
                apu.writeRegister(gbapu::Apu::REG_NR30, 0x80);
                retrigger = true;
            }

            // do nothing if there is no waveform in the list
        } else {
            // write envelope
            // [rNRx2] <- envelope
            apu.writeRegister(REGS_START + 2, state.envelope);
            retrigger = true;
        }

    }

    if (state.playing && writePanning) {
        // we can only update panning when the channel is playing
        // doing so otherwise may cause the note to start playing after being cut
        constexpr uint8_t panningMask = 0x11 << +ch;
        auto nr51 = apu.readRegister(gbapu::Apu::REG_NR51);
        nr51 &= ~panningMask; // clear current setting
        switch (state.panning) {
            case 0x00:
                // mute
                break;
            case 0x01:
                // left
                nr51 |= 0x10 << +ch;
                break;
            case 0x02:
                // right
                nr51 |= 0x01 << +ch;
                break;
            default:
                // middle
                nr51 |= 0x11 << +ch;
                break;

        }
        apu.writeRegister(gbapu::Apu::REG_NR51, nr51);
    }

    bool const timbreChanged = lastState.timbre != state.timbre;
    bool const freqChanged = lastState.frequency != state.frequency;

    if constexpr (ch == ChType::ch4) {

        // for CH4, timbre and frequency effects a single register
        if (timbreChanged || freqChanged) {

            uint8_t nr43;
            if (!freqChanged) {
                // only timbre changed, just set/clear the step-width bit
                nr43 = apu.readRegister(gbapu::Apu::REG_NR43);
                if (state.timbre) {
                    nr43 |= 0x08;
                } else {
                    nr43 &= ~0x08;
                }
            } else {
                // frequency and/or timbre changed
                nr43 = NoiseFrequencyControl::toNR43(state.frequency);
                if (state.timbre) {
                    nr43 |= 0x08;
                }

            }

            apu.writeRegister(gbapu::Apu::REG_NR43, nr43);
        }


        if (retrigger) {
            apu.writeRegister(gbapu::Apu::REG_NR44, 0x80);
        }
    } else {

        if (timbreChanged) {
            if constexpr (ch == ChType::ch3) {
                uint8_t vol;
                // wave volume
                switch (state.timbre) {
                    case 0x00:
                        // mute
                        vol = 0x00;
                        break;
                    case 0x01:
                        // 25%
                        vol = 0x60;
                        break;
                    case 0x02:
                        // 50%
                        vol = 0x40;
                        break;
                    default:
                        // 100%
                        vol = 0x20;
                        break;
                }
                apu.writeRegister(gbapu::Apu::REG_NR32, vol);

            } else {
                // duty
                uint8_t duty = (state.timbre & 3) << 6;
                apu.writeRegister(REGS_START + 1, duty);
            }
        }

        if (freqChanged) {
            apu.writeRegister(REGS_START + 3, state.frequency & 0xFF);
            uint8_t msb = state.frequency >> 8;
            if (retrigger) {
                msb |= 0x80;
            }
            apu.writeRegister(REGS_START + 4, msb);
        } else if (retrigger) {
            apu.writeRegister(REGS_START + 4, 0x80 | (state.frequency >> 8));
        }

    }





}

template <ChType ch>
void ChannelControl<ch>::clear(IApu &apu) noexcept {
    // clear registers
    constexpr uint8_t regno = gbapu::Apu::REG_NR10 + (+ch * GB_CHANNEL_REGS);
    for (uint8_t i = 0; i != GB_CHANNEL_REGS; ++i) {
        apu.writeRegister(regno + i, 0);
    }
    // clear terminals for the channel
    auto nr51 = apu.readRegister(gbapu::Apu::REG_NR51);
    nr51 &= ~((uint8_t)0x11 << (+ch));
    apu.writeRegister(gbapu::Apu::REG_NR51, nr51);
}

template <ChType ch>
void ChannelControl<ch>::init(IApu &apu, WaveformList const& waveList, ChannelState const& state) noexcept {
    ChannelState fakeLast = state;
    fakeLast.playing = !state.playing;
    fakeLast.envelope = ~state.envelope;
    fakeLast.panning = ~state.panning;
    fakeLast.timbre = ~state.timbre;
    fakeLast.frequency = ~state.frequency;
    update(apu, waveList, fakeLast, state);
}

template class ChannelControl<ChType::ch1>;
template class ChannelControl<ChType::ch2>;
template class ChannelControl<ChType::ch3>;
template class ChannelControl<ChType::ch4>;


}

//
//#include "trackerboy/engine/ChannelControl.hpp"
//
//#include "internal/enumutils.hpp"
//
//namespace trackerboy {
//
//ChannelControl::ChannelControl() :
//    mLocks()
//{
//}
//
//bool ChannelControl::isLocked(ChType ch) const noexcept {
//    return !mLocks.test(+ch);
//}
//
//void ChannelControl::lock(RuntimeContext const& rc, ChType ch) noexcept {
//    if (mLocks.test(+ch)) {
//        mLocks.reset(+ch);
//        // restore state
//    }
//}
//
//void ChannelControl::unlock(RuntimeContext const& rc, ChType ch) noexcept {
//    if (!mLocks.test(+ch)) {
//        mLocks.set(+ch);
//        // clear registers
//        uint8_t regno = gbapu::Apu::REG_NR10 + (+ch * GB_CHANNEL_REGS);
//        for (int i = 0; i != GB_CHANNEL_REGS; ++i) {
//            rc.apu.writeRegister(regno++, 0);
//        }
//        // clear terminals for the channel
//        auto nr51 = rc.apu.readRegister(gbapu::Apu::REG_NR51);
//        nr51 &= ~((uint8_t)0x11 << (+ch));
//        rc.apu.writeRegister(gbapu::Apu::REG_NR51, nr51);
//    }
//}
//
//template <ChType ch>
//void ChannelControl::update(gbapu::Apu &apu, WaveformList const& wavelist, ChannelState const& state) noexcept {
//    constexpr auto chint = +ch;
//
//    if (isLocked(ch)) {
//
//    }
//
//    mChannelState[chint] = state;
//}
//
//void ChannelControl::update(ChType ch, RuntimeContext const& rc, ChannelState const& state) noexcept {
//    //auto& lastState = mChannelState[+ch];
//
//
//    //// only apply changes to registers if the channel is locked
//    //if (isLocked(ch)) {
//    //    
//    //    bool retrigger = false;
//    //    auto channelRegs = gbapu::Apu::REG_NR10 + (+ch * GB_CHANNEL_REGS);
//    //    
//    //    if (lastState.playing != state.playing) {
//    //        if (state.playing) {
//    //            // note trigger
//    //            if (ch == ChType::ch3) {
//    //                // wave DAC on
//    //                rc.apu.writeRegister(gbapu::Apu::REG_NR30, 0x80);
//    //            }
//    //            retrigger = true;
//    //        } else {
//    //            // note cut
//    //            // we can cut a channel by disabling the DAC
//    //            if (ch == ChType::ch3) {
//    //                // wave DAC off
//    //                rc.apu.writeRegister(gbapu::Apu::REG_NR30, 0x00);
//    //            } else {
//    //                // set envelope to 0 (no retrigger needed)
//    //                rc.apu.writeRegister(channelRegs + 2, 0);
//    //            }
//    //        }
//    //    }
//
//    //    if (state.playing) {
//
//    //        if (retrigger || lastState.envelope != state.envelope) {
//    //            // changing the envelope requires a retrigger for all channels
//    //            // write the envelope
//    //            if (ch == ChType::ch3) {
//    //                auto waveform = rc.waveList[state.envelope];
//    //                if (waveform != nullptr) {
//    //                    writeWaveram(rc, *waveform);
//    //                }
//    //            } else {
//    //                // write envelope
//    //                // [rNRx2] <- envelope
//    //                rc.apu.writeRegister(gbapu::Apu::REG_NR12 + (static_cast<int>(ch) * GB_CHANNEL_REGS), envelope);
//    //            }
//    //            retrigger = true;
//    //        }
//
//    //        if (lastState.timbre != state.timbre) {
//    //            // write the timbre
//    //        }
//
//    //        if (lastState.panning != state.panning) {
//
//    //        }
//
//    //        bool const writeFreqLsb = lastState.frequency != state.frequency;
//    //        if (writeFreqLsb) {
//
//    //        }
//
//
//    //    }
//
//
//    //}
//
//    //// maintain the state of the channel
//    //lastState = state;
//}
//
//
//void ChannelControl::writeEnvelope(ChType ch, RuntimeContext &rc, uint8_t envelope, uint8_t freqMsb) {
//    if (ch == ChType::ch3) {
//
//        auto waveform = rc.waveList[envelope];
//        if (waveform == nullptr) {
//            return; // do nothing if no waveform exists
//        }
//
//        writeWaveram(rc, *waveform);
//
//
//    } else {
//        // write envelope
//        // [rNRx2] <- envelope
//        rc.apu.writeRegister(gbapu::Apu::REG_NR12 + (static_cast<int>(ch) * GB_CHANNEL_REGS), envelope);
//    }
//    // retrigger
//    rc.apu.writeRegister(gbapu::Apu::REG_NR14 + (static_cast<int>(ch) * GB_CHANNEL_REGS), 0x80 | freqMsb);
//}
//
//void ChannelControl::writeFrequency(ChType ch, RuntimeContext &rc, uint16_t frequency) {
//    auto lsbReg = (uint8_t)(gbapu::Apu::REG_NR13 + (static_cast<int>(ch) * GB_CHANNEL_REGS));
//    rc.apu.writeRegister(lsbReg++, (uint8_t)(frequency & 0xFF));
//    rc.apu.writeRegister(lsbReg, (uint8_t)(frequency >> 8));
//
//}
//
//void ChannelControl::writeTimbre(ChType ch, RuntimeContext &rc, uint8_t timbre) {
//    uint8_t reg = 0;
//    switch (ch) {
//        case ChType::ch1:
//            reg = gbapu::Apu::REG_NR11;
//            goto pulse;
//        case ChType::ch2:
//            reg = gbapu::Apu::REG_NR21;
//        pulse:
//            timbre <<= 6;
//            break;
//        case ChType::ch3:
//            reg = gbapu::Apu::REG_NR32;
//            // 0   1   2   3   : timbre
//            // 0% 25% 50% 100% : volume
//            // 0   3   2   1   : NR32
//            if (timbre == 1) {
//                timbre = 3;
//            } else if (timbre == 3) {
//                timbre = 1;
//            }
//            timbre <<= 5;
//            break;
//        case ChType::ch4:
//            reg = gbapu::Apu::REG_NR43;
//            uint8_t nr43 = rc.apu.readRegister(reg);
//            if (timbre) {
//                nr43 |= 0x08;
//            } else {
//                nr43 &= ~0x08;
//            }
//            timbre = nr43;
//            break;
//    }
//
//    rc.apu.writeRegister(reg, timbre);
//}
//
//void ChannelControl::writePanning(ChType ch, RuntimeContext &rc, uint8_t panning) {
//    uint8_t mask = 0x11 << static_cast<int>(ch);
//    uint8_t nr51 = rc.apu.readRegister(gbapu::Apu::REG_NR51);
//    nr51 = (nr51 & (~mask)) | (panning << static_cast<int>(ch));
//    rc.apu.writeRegister(gbapu::Apu::REG_NR51, nr51);
//}
//
//void ChannelControl::writeWaveram(RuntimeContext &rc, Waveform &waveform) {
//    // DAC OFF
//    rc.apu.writeRegister(gbapu::Apu::REG_NR30, 0x00);
//
//    // copy wave
//    auto data = waveform.data();
//    for (int i = 0; i != 16; ++i) {
//        rc.apu.writeRegister((uint8_t)(gbapu::Apu::REG_WAVERAM + i), data[i]);
//    }
//
//    // DAC ON
//    rc.apu.writeRegister(gbapu::Apu::REG_NR30, 0x80);
//}
//
//
//}
