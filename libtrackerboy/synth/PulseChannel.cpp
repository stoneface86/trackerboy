
#include "trackerboy/synth/PulseChannel.hpp"

#define DUTY_SIZE 8

namespace trackerboy {

static uint8_t DUTY_TABLE[][DUTY_SIZE] = {
    // Duty 12.5%:  00000001
    {
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MAX
    },
    // Duty 25%:    10000001
    {
        Gbs::SAMPLE_MAX,
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MAX
    },
    // Duty 50%:    10000111
    {
        Gbs::SAMPLE_MAX,
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MAX,
        Gbs::SAMPLE_MAX,
        Gbs::SAMPLE_MAX
    },
    // Duty 75%:    01111110
    {
        Gbs::SAMPLE_MIN,
        Gbs::SAMPLE_MAX,
        Gbs::SAMPLE_MAX,
        Gbs::SAMPLE_MAX,
        Gbs::SAMPLE_MAX,
        Gbs::SAMPLE_MAX,
        Gbs::SAMPLE_MAX,
        Gbs::SAMPLE_MIN
    }
};

PulseChannel::PulseChannel() : 
    EnvChannel(),
    mDuty(Gbs::DEFAULT_DUTY),
    mDutyCounter(0)
{        
}

void PulseChannel::reset() {
    EnvChannel::reset();
    mFreqCounter = 0;
    mDutyCounter = 0;
}

void PulseChannel::setDuty(Gbs::Duty _duty) {
    mDuty = _duty;
}

void PulseChannel::step(unsigned cycles) {
    mFreqCounter += cycles;
    unsigned dutysteps = mFreqCounter / mFreqCounterMax;
    mFreqCounter %= mFreqCounterMax;
    mDutyCounter = (mDutyCounter + dutysteps) & 0x7; // & 7 == % 8
    mCurrentSample = DUTY_TABLE[static_cast<uint8_t>(mDuty)][mDutyCounter];
}
}