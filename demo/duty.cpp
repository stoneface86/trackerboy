#include "base.h"

#include <iostream>

#define DEMO_RUNTIME 1000
#define DEMO_FREQ    440.0f

using namespace trackerboy;

namespace demo {

    static const char* DUTY_STRINGS[] = {
        "12.5",
        "25.0",
        "50.0",
        "75.0"
    };

    int init(Synth &synth) {
        Mixer &mixer = synth.getMixer();
        PulseChannel &ch2 = synth.getChannels().ch2;

        ch2.setEnvStep(MAX_ENV_STEPS);
        ch2.setFrequency(DEMO_FREQ);
        ch2.reset();

        mixer.setTerminalEnable(TERM_LEFT, true);
        mixer.setTerminalEnable(TERM_RIGHT, true);
        mixer.setEnable(OUT_SOUND2_BOTH);
        return NOERROR;
    }

    int run(int cont, Synth &synth, long &runtime) {
        if (cont > 3) {
            runtime = 0;
        } else {
            Duty duty = (Duty)cont;
            std::cout << "Duty: " << DUTY_STRINGS[duty] << "%" << std::endl;
            synth.getChannels().ch2.setDuty(duty);
            runtime = DEMO_RUNTIME;
        }
        return NOERROR;
    }

}