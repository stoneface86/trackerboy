
#include "trackerboy/synth/PulseOsc.hpp"
#include "./sampletable.hpp"

namespace {

const uint8_t DUTY_WAVEFORMS[4][4] = {
    // 12.5%
    { 0x00, 0x00, 0x00, 0x0F },
    // 25.0%
    { 0xF0, 0x00, 0x00, 0x0F },
    // 50%
    { 0xF0, 0x00, 0x0F, 0xFF },
    // 75%
    { 0x0F, 0xFF, 0xFF, 0xF0 }
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
        mDuty = duty;
        setDeltaBuf();
    }
    
}

void PulseOsc::setEnvelope(uint8_t envelope) {

}


void PulseOsc::setDeltaBuf() {

    deltaSet(DUTY_WAVEFORMS[static_cast<size_t>(mDuty)]);
}



}