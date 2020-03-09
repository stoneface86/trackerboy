
#include <iostream>

#include "../demo.hpp"

#include "trackerboy/synth/utils.hpp"

#define DEMO_RUNTIME 1000
#define DEMO_FREQ    toGbFreq(440.0f)

using namespace trackerboy;

static const char* DUTY_STRINGS[] = {
    "12.5",
    "25.0",
    "50.0",
    "75.0"
};

DutyDemo::DutyDemo() :
    Demo("Duty")
{
}


void DutyDemo::init(Synth &synth) {
    Mixer &mixer = synth.mixer();
    //PulseChannel &ch2 = synth.getChannels().ch2;
    PulseOsc &osc2 = synth.hardware().osc2;
    Envelope &env2 = synth.hardware().env2;

    //ch2.setEnvStep(Gbs::MAX_ENV_STEPS);
    env2.setRegister(Gbs::MAX_ENV_STEPS << 4);
    osc2.setFrequency(DEMO_FREQ);
    osc2.reset();

    mixer.setEnable(Gbs::OUT_BOTH2);
}

long DutyDemo::setupNextRun(Synth &synth, unsigned counter) {
    if (counter > 3) {
        return 0;
    } else {
        std::cout << "Duty: " << DUTY_STRINGS[counter] << "%" << std::endl;
        synth.hardware().osc2.setDuty(static_cast<Gbs::Duty>(counter));
        return DEMO_RUNTIME;
    }
}
