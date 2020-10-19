
#include "trackerboy/engine/ChannelControl.hpp"


namespace trackerboy {

ChannelControl::ChannelControl() :
    mLocks(0)
{
}

bool ChannelControl::isLocked(ChType ch) const noexcept {
    return !(mLocks & (1 << static_cast<int>(ch)));
}

uint8_t ChannelControl::lockbits() const noexcept {
    return mLocks & 0xF;
}

void ChannelControl::lock(ChType ch) noexcept {
    mLocks &= ~(1 << static_cast<int>(ch));
}

void ChannelControl::unlock(ChType ch) noexcept {
    mLocks |= (1 << static_cast<int>(ch));
}

void ChannelControl::writeEnvelope(ChType ch, RuntimeContext &rc, uint8_t envelope, uint8_t freqMsb) {
    if (ch == ChType::ch3) {

        Waveform *waveform = rc.waveTable[envelope];
        if (waveform == nullptr) {
            return; // do nothing if no waveform exists
        }
        // on real hardware we would have to disable the generator by
        // writing 0x00 to NR30 to prevent corruption. The emulator does
        // not corrupt wave ram on retrigger so we can skip this step.

        // copy waveform to wave ram
        rc.synth.setWaveram(*waveform);

    } else {
        // write envelope
        // [rNRx2] <- envelope
        rc.synth.writeRegister(Gbs::REG_NR12 + (static_cast<int>(ch) * Gbs::REGS_PER_CHANNEL), envelope);
    }
    // retrigger
    rc.synth.writeRegister(Gbs::REG_NR14 + (static_cast<int>(ch) * Gbs::REGS_PER_CHANNEL), 0x80 | freqMsb);
}

void ChannelControl::writeFrequency(ChType ch, RuntimeContext &rc, uint16_t frequency) {
    uint16_t lsbReg = Gbs::REG_NR13 + (static_cast<int>(ch) * Gbs::REGS_PER_CHANNEL);
    rc.synth.writeRegister(lsbReg++, frequency & 0xFF);
    rc.synth.writeRegister(lsbReg, frequency >> 8);

}

void ChannelControl::writeTimbre(ChType ch, RuntimeContext &rc, uint8_t timbre) {
    uint16_t reg;
    switch (ch) {
        case ChType::ch1:
            reg = Gbs::REG_NR11;
            goto pulse;
        case ChType::ch2:
            reg = Gbs::REG_NR21;
        pulse:
            timbre <<= 6;
            break;
        case ChType::ch3:
            reg = Gbs::REG_NR32;
            // 0   1   2   3   : timbre
            // 0% 25% 50% 100% : volume
            // 0   3   2   1   : NR32
            if (timbre == 1) {
                timbre = 3;
            } else if (timbre == 3) {
                timbre = 1;
            }
            timbre <<= 5;
            break;
        case ChType::ch4:
            reg = Gbs::REG_NR43;
            uint8_t nr43 = rc.synth.readRegister(reg);
            if (timbre) {
                nr43 |= 0x08;
            } else {
                nr43 &= ~0x08;
            }
            timbre = nr43;
            break;
    }

    rc.synth.writeRegister(reg, timbre);
}



}
