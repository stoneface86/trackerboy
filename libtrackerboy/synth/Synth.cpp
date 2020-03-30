
#include "trackerboy/synth/Synth.hpp"
#include "trackerboy/gbs.hpp"

#include <algorithm>
#include <climits>
#include <cmath>

namespace {

constexpr unsigned CYCLES_PER_TRIGGER = 8192;

}


namespace trackerboy {

// Frame sequencer
// this part of the APU controls when components such as sweep, envelope
// and length counters are triggered. The sequencer itself is stepped every
// 8192 cycles or at 512 Hz. The table below shows which steps the components
// are triggered (except for length counters).
//
// Step:                 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
// --------------------------+---+---+---+---+---+---+---+-------------------
// Sweep        (128 Hz) |         x               x       
// envelope     ( 64 Hz) |                             x 
//
// The synthesizer emulates this by generating samples before each trigger,
// applying the trigger, and then repeating until the buffer is filled.
//
// The time between each trigger is stored in the mTriggerTimings array. For
// the first element, this value is the number of samples needed until the
// first trigger (which is Sweep @ step 2).
//

Synth::TriggerType const Synth::TRIGGER_SEQUENCE[] = {
    TriggerType::SWEEP,     // Sweep    @ step 2
    TriggerType::SWEEP,     // Sweep    @ step 6
    TriggerType::ENV        // Envelope @ step 7
};


Synth::Synth(float samplingRate, float framerate) :
    mSamplingRate(samplingRate),
    mFramerate(framerate),
    mHf(),
    mMixer(samplingRate),
    mCyclesPerSample(Gbs::CLOCK_SPEED / samplingRate),
    mCyclesPerFrame(Gbs::CLOCK_SPEED / framerate),
    mCycleOffset(0.0f),
    mFrameBuf(static_cast<size_t>(samplingRate / framerate)),
    mTriggerTimings{
        (CYCLES_PER_TRIGGER * 3) / mCyclesPerSample, // 3 steps from 7 to 2
        (CYCLES_PER_TRIGGER * 4) / mCyclesPerSample, // 4 steps from 2 to 6
        (CYCLES_PER_TRIGGER * 1) / mCyclesPerSample  // 1 step from  6 to 7
    },
    mSampleCounter(0.0f),
    mSamplesToTrigger(mTriggerTimings[0]),
    mTriggerIndex(0),
    mOutputStat(Gbs::OUT_ALL)
{
    // NOTE: mTriggerTimings will need to be recalculated if the sampling rate
    // changes. Currently there is no way to change it after construction,
    // although this may change in the future.
}

float* Synth::buffer() {
    return mFrameBuf.data();
}

HardwareFile& Synth::hardware() {
    return mHf;
}

void Synth::fill(float buf[], size_t nsamples) {
    // TODO: implement later
}

size_t Synth::run() {
    // TODO: implement later
    return 0;

}

uint8_t Synth::readRegister(uint16_t addr) {
    
    /*
    Read masks

         NRx0 NRx1 NRx2 NRx3 NRx4
        ---------------------------
    NR1x  $80  $3F $00  $FF  $BF 
    NR2x  $FF  $3F $00  $FF  $BF 
    NR3x  $7F  $FF $9F  $FF  $BF 
    NR4x  $FF  $FF $00  $00  $BF 
    NR5x  $00  $00 $70

    $FF27-$FF2F always read back as $FF
    */

    #define readDuty(gen) (0x3F | (gen.duty() << 6))

    switch (addr) {
        case Gbs::REG_NR10:
            return mHf.sweep1.readRegister();
        case Gbs::REG_NR11:
            return readDuty(mHf.gen1);
        case Gbs::REG_NR12:
            return mHf.env1.readRegister();
        case Gbs::REG_NR13:
            return 0xFF;
        case Gbs::REG_NR14:
            return 0xBF;
        case Gbs::REG_NR21:
            return readDuty(mHf.gen2);
        case Gbs::REG_NR22:
            return mHf.env2.readRegister();
        case Gbs::REG_NR23:
            return 0xFF;
        case Gbs::REG_NR24:
            return 0xBF;
        case Gbs::REG_NR30:
            // TODO
            return 0x7F;
        case Gbs::REG_NR31:
            return 0xFF;
        case Gbs::REG_NR32:
            return 0x9F | (mHf.gen3.volume());
        case Gbs::REG_NR33:
            return 0xFF;
        case Gbs::REG_NR34:
            return 0xBF;
        case Gbs::REG_NR41:
            return 0xBF;
        case Gbs::REG_NR42:
            return mHf.env4.readRegister();
        case Gbs::REG_NR43:
            return mHf.gen4.readRegister();
        case Gbs::REG_NR44:
            return 0xBF;
        case Gbs::REG_NR50:
            return 0;
        case Gbs::REG_NR51:
            return mOutputStat;
        case Gbs::REG_NR52:
            return 0x70;
        default:
            return 0xFF;


    }
}


void Synth::reset() {
    // TODO: implement later
}


void Synth::restart(ChType ch) {
    switch (ch) {
        case ChType::ch1:
            mHf.env1.restart();
            mHf.sweep1.restart();
            mHf.gen1.restart();
            break;
        case ChType::ch2:
            mHf.env2.restart();
            mHf.gen2.restart();
            break;
        case ChType::ch3:
            mHf.gen3.restart();
            break;
        case ChType::ch4:
            mHf.env4.restart();
            mHf.gen4.restart();
            break;
    }
}


void Synth::setOutputEnable(Gbs::OutputFlags flags) {
    mOutputStat = flags;
}

void Synth::setOutputEnable(ChType ch, Gbs::Terminal term, bool enabled) {
    uint8_t flag = 0;
    if (term & Gbs::TERM_LEFT) {
        flag = 1 << static_cast<uint8_t>(ch);
    }

    if (term & Gbs::TERM_RIGHT) {
        flag |= 16 << static_cast<uint8_t>(ch);
    }

    if (enabled) {
        mOutputStat |= flag;
    } else {
        mOutputStat &= ~flag;
    }
}

void Synth::writeRegister(uint16_t addr, uint8_t value) {

    #define writeDuty(gen) gen.setDuty(static_cast<Gbs::Duty>(value >> 6))
    #define writeFreqLSB(gen) gen.setFrequency((gen.frequency() & 0xF0) | value)
    #define writeFreqMSB(gen) gen.setFrequency((gen.frequency() & 0x0F) | (value << 8))
    #define trigger(ch) if (value & 0x80) restart(ch)

    switch (addr) {
        case Gbs::REG_NR10:
            mHf.sweep1.writeRegister(value);
            break;
        case Gbs::REG_NR11:
            writeDuty(mHf.gen1);
            // length counters aren't implemented so ignore the other 6 bits
            break;
        case Gbs::REG_NR12:
            mHf.env1.writeRegister(value);
            break;
        case Gbs::REG_NR13:
            writeFreqLSB(mHf.gen1);
            break;
        case Gbs::REG_NR14:
            writeFreqMSB(mHf.gen1);
            trigger(ChType::ch1);
            break;
        case Gbs::REG_NR21:
            writeDuty(mHf.gen2);
            break;
        case Gbs::REG_NR22:
            mHf.env2.writeRegister(value);
            break;
        case Gbs::REG_NR23:
            writeFreqLSB(mHf.gen2);
            break;
        case Gbs::REG_NR24:
            writeFreqMSB(mHf.gen2);
            trigger(ChType::ch2);
            break;
        case Gbs::REG_NR30:
            // TODO: implement functionality in WaveGen
            break;
        case Gbs::REG_NR31:
            // no length counter so do nothing
            break;
        case Gbs::REG_NR32:
            mHf.gen3.setVolume(static_cast<Gbs::WaveVolume>((value >> 5) & 0x3));
            break;
        case Gbs::REG_NR33:
            writeFreqLSB(mHf.gen3);
            break;
        case Gbs::REG_NR34:
            writeFreqMSB(mHf.gen3);
            trigger(ChType::ch3);
            break;
        case Gbs::REG_NR41:
            // no length counter do nothing
            break;
        case Gbs::REG_NR42:
            mHf.env4.writeRegister(value);
            break;
        case Gbs::REG_NR43:
            mHf.gen4.writeRegister(value);
            break;
        case Gbs::REG_NR44:
            trigger(ChType::ch4);
            break;
        case Gbs::REG_NR50:
            // not implemented, do nothing
            break;
        case Gbs::REG_NR51:
            mOutputStat = value;
            break;
        case Gbs::REG_NR52:
            // not implemented
            break;
        default:
            break;
    }
}

}
