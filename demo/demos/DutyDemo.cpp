
#include <iostream>

#include "../demo.hpp"

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
    Mixer &mixer = synth.getMixer();
    PulseChannel &ch2 = synth.getChannels().ch2;

    ch2.setEnvStep(MAX_ENV_STEPS);
    ch2.setFrequency(DEMO_FREQ);
    ch2.reset();

    mixer.setTerminalEnable(Terminal::left, true);
    mixer.setTerminalEnable(Terminal::right, true);
    mixer.setEnable(OutputFlags::both2);
}

long DutyDemo::setupNextRun(Synth &synth, unsigned counter) {
    if (counter > 3) {
        return 0;
    } else {
        std::cout << "Duty: " << DUTY_STRINGS[counter] << "%" << std::endl;
        synth.getChannels().ch2.setDuty(static_cast<Duty>(counter));
        return DEMO_RUNTIME;
    }
}