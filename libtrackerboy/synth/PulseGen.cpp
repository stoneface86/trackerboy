
#include "trackerboy/synth/PulseGen.hpp"

namespace {

// multiplier for frequency calculation
// 64 Hz - 131.072 KHz
static constexpr unsigned PULSE_MULTIPLIER = 4;

//                    STEP: 76543210
// Bits 24-31 - 75%   Duty: 01111110 (0x7E) _------_
// Bits 16-23 - 50%   Duty: 11100001 (0xE1) -____---
// Bits  8-15 - 25%   Duty: 10000001 (0x81) -______-
// Bits  0-7  - 12.5% Duty: 10000000 (0x80) _______-

static constexpr uint32_t DUTY_MASK = 0x7EE18180;

}


namespace trackerboy {


PulseGen::PulseGen() :
    mFrequency(Gbs::DEFAULT_FREQUENCY),
    mDuty(Gbs::DEFAULT_DUTY),
    mFreqCounter(0),
    mDutyCounter(0),
    mPeriod((2048 - mFrequency) * PULSE_MULTIPLIER)
{
}

unsigned PulseGen::remainder() {
    return mPeriod - mFreqCounter;
}

void PulseGen::restart() {
    mFreqCounter = 0;
    mDutyCounter = 0;
}

void PulseGen::setDuty(Gbs::Duty duty) {
    mDuty = duty;
}

void PulseGen::setFrequency(uint16_t frequency) {
    frequency &= Gbs::MAX_FREQUENCY;
    mFrequency = frequency;
    mPeriod = (2048 - mFrequency) * PULSE_MULTIPLIER;
}

uint8_t PulseGen::step(unsigned cycles) {
    // this implementation uses bit shifting instead of a lookup table

    // advance the counter
    mFreqCounter += cycles;
    // number of duty steps to cycle through
    unsigned dutysteps = mFreqCounter / mPeriod;
    mFreqCounter %= mPeriod;
    mDutyCounter = (mDutyCounter + dutysteps) & 0x7; // & 7 == % 8

    // DUTY_MASK contains all duty waveforms
    // first byte is 12.5% second is 25% and so on
    unsigned shift = (static_cast<uint8_t>(mDuty) << 3) + mDutyCounter;
    return (DUTY_MASK >> shift) & 1;
}



}
