
#include "gbsynth.hpp"

#define DUTY_SIZE 8

namespace gbsynth {

static uint8_t DUTY_TABLE[][DUTY_SIZE] = {
    // Duty 12.5%:  00000001
    {SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MAX},
    // Duty 25%:    10000001
    {SAMPLE_MAX, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MAX},
    // Duty 50%:    10000111
    {SAMPLE_MAX, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MIN, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MAX},
    // Duty 75%:    01111110
    {SAMPLE_MIN, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MAX, SAMPLE_MIN}
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