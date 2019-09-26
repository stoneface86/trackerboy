
#include "trackerboy/synth.hpp"

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
    duty(Gbs::DEFAULT_DUTY),
    dutyCounter(0)
{        
}

void PulseChannel::reset() {
    EnvChannel::reset();
    freqCounter = 0;
    dutyCounter = 0;
}

void PulseChannel::setDuty(Gbs::Duty _duty) {
    duty = _duty;
}

void PulseChannel::step(unsigned cycles) {
    freqCounter += cycles;
    unsigned dutysteps = freqCounter / freqCounterMax;
    freqCounter %= freqCounterMax;
    dutyCounter = (dutyCounter + dutysteps) & 0x7; // & 7 == % 8
    currentSample = DUTY_TABLE[static_cast<uint8_t>(duty)][dutyCounter];
}
}