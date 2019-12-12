
#include "trackerboy/synth/PulseOsc.hpp"
#include "./sampletable.hpp"

namespace {

const uint8_t DUTY_WAVEFORMS[] = {
    // 12.5%
    0x00, 0x00, 0x00, 0x0F,
    // 25.0%
    0xF0, 0x00, 0x00, 0x0F,
    // 50%
    0xF0, 0x00, 0x0F, 0xFF,
    // 75%
    0x0F, 0xFF, 0xFF, 0xF0
};


}


namespace trackerboy {


PulseOsc::PulseOsc(float samplingRate) :
    Osc(samplingRate, 4, 8),
    mDuty(Gbs::DEFAULT_DUTY),
    mEnvelope(Gbs::DEFAULT_ENV_STEPS)
{
    setDeltaBuf();
}

void PulseOsc::setDuty(Gbs::Duty duty) {
    if (duty != mDuty) {
        setDeltaBuf();
    }
    
}

void PulseOsc::setEnvelope(uint8_t envelope) {

}


void PulseOsc::setDeltaBuf() {

    uint8_t waveform[];
    switch (duty) {
        case Gbs::DUTY_125:
            waveform = DUTY_WAVEFORMS;
            break;
        case Gbs::DUTY_25:
            waveform = DUTY_WAVEFORMS + 4;
            break;
        case Gbs::DUTY_50:
            waveform = DUTY_WAVEFORMS + 8;
            break;
        case Gbs::DUTY_75:
            waveform = DUTY_WAVEFORMS + 12;
            break;
    }

    deltaSet(waveform);
}



}