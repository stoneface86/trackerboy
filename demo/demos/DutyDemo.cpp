
#include "demo.hpp"

#define DEMO_RUNTIME 1000
#define DEMO_FREQ    440.0f

using namespace gbsynth;

static const char* DUTY_STRINGS[] = {
    "12.5",
    "25.0",
    "50.0",
    "75.0"
};

void DutyDemo::init(Synth &synth) {
    Mixer &mixer = synth.getMixer();
    PulseChannel &ch2 = synth.getChannels().ch2;

    ch2.setEnvStep(MAX_ENV_STEPS);
    ch2.setFrequency(DEMO_FREQ);
    ch2.reset();

    mixer.setTerminalEnable(TERM_LEFT, true);
    mixer.setTerminalEnable(TERM_RIGHT, true);
    mixer.setEnable(OUT_SOUND2_BOTH);
}

long DutyDemo::setupNextRun(Synth &synth, unsigned counter) {
    if (counter > 3) {
        return 0;
    } else {
        Duty duty = (Duty)counter;
        std::cout << "Duty: " << DUTY_STRINGS[duty] << "%" << std::endl;
        synth.getChannels().ch2.setDuty(duty);
        return DEMO_RUNTIME;
    }
}