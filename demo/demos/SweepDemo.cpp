
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
    Mixer &mixer = synth.mixer();
    HardwareFile &hf = synth.hardware();

    //cf.ch1.setEnvStep(Gbs::MAX_ENV_STEPS);
    hf.env1.setRegister(Gbs::MAX_ENV_STEPS);

    hf.sweep1.setRegister((DEMO_SWEEP_SHIFT << 4) | DEMO_SWEEP_TIME);

    mixer.setEnable(Gbs::OUT_BOTH1);
}

long SweepDemo::setupNextRun(Synth &synth, unsigned counter) {

    HardwareFile &hf = synth.hardware();

    uint16_t freq;
    Gbs::SweepMode mode;

    switch (counter) {
        case 0:
            // sweep subtraction for 4 seconds
            freq = DEMO_SUB_FREQ;
            mode = Gbs::SWEEP_SUBTRACTION;
            std::cout << "Sweep subtraction" << std::endl;
            break;
        case 1:
            // sweep addition for 4 seconds
            freq = DEMO_ADD_FREQ;
            mode = Gbs::SWEEP_ADDITION;
            std::cout << "Sweep addition" << std::endl;
            break;
        default:
            // we are done when counter > 1
            return 0;
    }


    hf.osc1.setFrequency(freq);
    hf.sweep1.setSweepMode(mode);
    ch1.reset();

    return DEMO_RUNTIME;
}
