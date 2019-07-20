
#include <iostream>
#include "../demo.hpp"

#define DEMO_RUNTIME 4000
#define DEMO_SWEEP_SHIFT 7
#define DEMO_SWEEP_TIME 1
#define DEMO_SUB_FREQ 2000
#define DEMO_ADD_FREQ 256

using namespace gbsynth;

void SweepDemo::init(Synth &synth) {
    ChannelFile &cf = synth.getChannels();
    Mixer &mixer = synth.getMixer();
    Sweep &sweep = synth.getSweep();

    cf.ch1.setEnvStep(MAX_ENV_STEPS);

    sweep.setSweepShift(DEMO_SWEEP_SHIFT);
    sweep.setSweepTime(DEMO_SWEEP_TIME);

    mixer.setTerminalEnable(TERM_LEFT, true);
    mixer.setTerminalEnable(TERM_RIGHT, true);
    mixer.setEnable(OUT_SOUND1_BOTH);
}

long SweepDemo::setupNextRun(Synth &synth, unsigned counter) {

    Sweep &sweep = synth.getSweep();
    PulseChannel &ch1 = synth.getChannels().ch1;

    uint16_t freq;
    SweepMode mode;

    switch (counter) {
        case 0:
            // sweep subtraction for 4 seconds
            freq = DEMO_SUB_FREQ;
            mode = SWEEP_SUBTRACTION;
            std::cout << "Sweep subtraction" << std::endl;
            break;
        case 1:
            // sweep addition for 4 seconds
            freq = DEMO_ADD_FREQ;
            mode = SWEEP_ADDITION;
            std::cout << "Sweep addition" << std::endl;
            break;
        default:
            // we are done when counter > 1
            return 0;
    }


    ch1.setFrequency(freq);
    ch1.reset();
    sweep.setSweepMode(mode);
    sweep.reset();

    return DEMO_RUNTIME;
}
