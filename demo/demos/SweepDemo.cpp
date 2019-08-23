
#include <iostream>
#include "../demo.hpp"

#define DEMO_RUNTIME 4000
#define DEMO_SWEEP_SHIFT 7
#define DEMO_SWEEP_TIME 1
#define DEMO_SUB_FREQ 2000
#define DEMO_ADD_FREQ 256

using namespace trackerboy;

SweepDemo::SweepDemo() :
    Demo("Sweep")
{
}

void SweepDemo::init(Synth &synth) {
    ChannelFile &cf = synth.getChannels();
    Mixer &mixer = synth.getMixer();

    cf.ch1.setEnvStep(MAX_ENV_STEPS);

    cf.ch1.setSweepShift(DEMO_SWEEP_SHIFT);
    cf.ch1.setSweepTime(DEMO_SWEEP_TIME);

    mixer.setTerminalEnable(Terminal::left, true);
    mixer.setTerminalEnable(Terminal::right, true);
    mixer.setEnable(Mixer::both1);
}

long SweepDemo::setupNextRun(Synth &synth, unsigned counter) {

    SweepPulseChannel &ch1 = synth.getChannels().ch1;

    uint16_t freq;
    SweepMode mode;

    switch (counter) {
        case 0:
            // sweep subtraction for 4 seconds
            freq = DEMO_SUB_FREQ;
            mode = SweepMode::subtraction;
            std::cout << "Sweep subtraction" << std::endl;
            break;
        case 1:
            // sweep addition for 4 seconds
            freq = DEMO_ADD_FREQ;
            mode = SweepMode::addition;
            std::cout << "Sweep addition" << std::endl;
            break;
        default:
            // we are done when counter > 1
            return 0;
    }


    ch1.setFrequency(freq);
    ch1.setSweepMode(mode);
    ch1.reset();

    return DEMO_RUNTIME;
}
