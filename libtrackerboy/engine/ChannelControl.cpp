
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

void ChannelControl::writeEnvelope(ChType ch, RuntimeContext &rc, uint8_t envelope) {
    if (ch == ChType::ch3) {

        Waveform *waveform = rc.waveTable[envelope];
        if (waveform == nullptr) {
            return; // do nothing if no waveform exists
        }
        // on real hardware we would have to disable the generator by
        // writing 0x00 to NR30 to prevent corruption. The emulator does
        // not corrupt wave ram on retrigger so we can skip this step.

        // copy waveform to wave ram
        rc.synth.hardware().gen3.copyWave(*waveform);

    } else {
        static constexpr uint16_t NRx2[] = { Gbs::REG_NR12, Gbs::REG_NR22, 0, Gbs::REG_NR42 };
        // write envelope
        // [rNRx2] <- envelope
        rc.synth.writeRegister(NRx2[static_cast<int>(ch)], envelope);
    }
    // retrigger
    rc.synth.restart(ch);
}

void ChannelControl::writeTimbre(ChType ch, RuntimeContext &rc, uint8_t timbre) {
    auto &hf = rc.synth.hardware();

    switch (ch) {
        case ChType::ch1:
        case ChType::ch2:
            PulseGen *gen;
            if (ch == ChType::ch1) {
                gen = &hf.gen1;
            } else {
                gen = &hf.gen2;
            }

            gen->setDuty(static_cast<Gbs::Duty>(timbre));
            break;
        case ChType::ch3:
            if (timbre == 1) {
                timbre = 3;
            } else if (timbre == 3) {
                timbre = 1;
            }
            hf.gen3.setVolume(static_cast<Gbs::WaveVolume>(timbre));
            break;
        case ChType::ch4:
            uint8_t nr43 = hf.gen4.readRegister();
            if (timbre) {
                nr43 |= 0x08;
            } else {
                nr43 &= ~0x08;
            }
            hf.gen4.writeRegister(nr43);
            break;
    }
}



}
