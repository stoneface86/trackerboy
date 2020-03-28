
#include <iostream>
#include "../demo.hpp"

#define DEMO_RUNTIME 4000
#define DEMO_SWEEP_SHIFT 7
#define DEMO_SWEEP_TIME 1
#define DEMO_SUB_FREQ 2030
#define DEMO_ADD_FREQ 256

using namespace trackerboy;

SweepDemo::SweepDemo() :
    Demo("Sweep")
{
}

void SweepDemo::init(Synth &synth) {
    HardwareFile &hf = synth.hardware();

    hf.env1.setRegister(0xF0);

    hf.sweep1.setRegister((DEMO_SWEEP_SHIFT << 4) | DEMO_SWEEP_TIME);

    synth.setOutputEnable(Gbs::OUT_BOTH1);
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


    hf.gen1.setFrequency(freq);
    hf.sweep1.setRegister((DEMO_SWEEP_TIME << 4) | (static_cast<uint8_t>(mode) << 3) | DEMO_SWEEP_SHIFT);
    synth.restart(ChType::ch1);

    return DEMO_RUNTIME;
}
