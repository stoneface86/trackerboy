
#include "trackerboy/synth.hpp"

#define DUTY_SIZE 8

namespace trackerboy {

static uint8_t DUTY_TABLE[][DUTY_SIZE] = {
    // Duty 12.5%:  00000001
    {
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MAX
    },
    // Duty 25%:    10000001
    {
        Channel::SAMPLE_MAX,
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MAX
    },
    // Duty 50%:    10000111
    {
        Channel::SAMPLE_MAX,
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MAX,
        Channel::SAMPLE_MAX,
        Channel::SAMPLE_MAX
    },
    // Duty 75%:    01111110
    {
        Channel::SAMPLE_MIN,
        Channel::SAMPLE_MAX,
        Channel::SAMPLE_MAX,
        Channel::SAMPLE_MAX,
        Channel::SAMPLE_MAX,
        Channel::SAMPLE_MAX,
        Channel::SAMPLE_MAX,
        Channel::SAMPLE_MIN
    }
};

PulseChannel::PulseChannel() : 
    EnvChannel(),
    FreqChannel(),
    duty((Duty)DEFAULT_DUTY),
    dutyCounter(0)
{        
}

void PulseChannel::reset() {
    EnvChannel::reset();
    freqCounter = 0;
    dutyCounter = 0;
}

void PulseChannel::setDuty(Duty _duty) {
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