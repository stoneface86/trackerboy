
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

        Waveform *waveform = rc.waveList[envelope];
        if (waveform == nullptr) {
            return; // do nothing if no waveform exists
        }

        writeWaveram(rc, *waveform);


    } else {
        // write envelope
        // [rNRx2] <- envelope
        rc.apu.writeRegister(gbapu::Apu::REG_NR12 + (static_cast<int>(ch) * Gbs::REGS_PER_CHANNEL), envelope);
    }
    // retrigger
    rc.apu.writeRegister(gbapu::Apu::REG_NR14 + (static_cast<int>(ch) * Gbs::REGS_PER_CHANNEL), 0x80 | freqMsb);
}

void ChannelControl::writeFrequency(ChType ch, RuntimeContext &rc, uint16_t frequency) {
    auto lsbReg = (uint8_t)(gbapu::Apu::REG_NR13 + (static_cast<int>(ch) * Gbs::REGS_PER_CHANNEL));
    rc.apu.writeRegister(lsbReg++, (uint8_t)(frequency & 0xFF));
    rc.apu.writeRegister(lsbReg, (uint8_t)(frequency >> 8));

}

void ChannelControl::writeTimbre(ChType ch, RuntimeContext &rc, uint8_t timbre) {
    uint8_t reg = 0;
    switch (ch) {
        case ChType::ch1:
            reg = gbapu::Apu::REG_NR11;
            goto pulse;
        case ChType::ch2:
            reg = gbapu::Apu::REG_NR21;
        pulse:
            timbre <<= 6;
            break;
        case ChType::ch3:
            reg = gbapu::Apu::REG_NR32;
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
            reg = gbapu::Apu::REG_NR43;
            uint8_t nr43 = rc.apu.readRegister(reg);
            if (timbre) {
                nr43 |= 0x08;
            } else {
                nr43 &= ~0x08;
            }
            timbre = nr43;
            break;
    }

    rc.apu.writeRegister(reg, timbre);
}

void ChannelControl::writePanning(ChType ch, RuntimeContext &rc, uint8_t panning) {
    uint8_t mask = 0x11 << static_cast<int>(ch);
    uint8_t nr51 = rc.apu.readRegister(gbapu::Apu::REG_NR51);
    nr51 = (nr51 & (~mask)) | (panning << static_cast<int>(ch));
    rc.apu.writeRegister(gbapu::Apu::REG_NR51, nr51);
}

void ChannelControl::writeWaveram(RuntimeContext &rc, Waveform &waveform) {
    // DAC OFF
    rc.apu.writeRegister(gbapu::Apu::REG_NR30, 0x00);

    // copy wave
    auto data = waveform.data();
    for (int i = 0; i != 16; ++i) {
        rc.apu.writeRegister((uint8_t)(gbapu::Apu::REG_WAVERAM + i), data[i]);
    }

    // DAC ON
    rc.apu.writeRegister(gbapu::Apu::REG_NR30, 0x80);
}


}
