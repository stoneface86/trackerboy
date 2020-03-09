
#include <iostream>

#include "../demo.hpp"

#define N_RUNS 4
#define DEMO_RUNTIME 1000

using namespace trackerboy;

static const char *VOL_STRINGS[] = {
    "mute",
    "100%",
    "50%",
    "25%"
};


WaveVolDemo::WaveVolDemo() :
    Demo::Demo("Wave volume")
{
}


void WaveVolDemo::init(Synth &synth) {
    Mixer &mixer = synth.mixer();
    WaveChannel &ch3 = synth.getChannels().ch3;

    ch3.setWaveform(WAVEDATA_TRIANGLE);
    ch3.setFrequency(1045);

    mixer.setTerminalEnable(Gbs::TERM_BOTH, true);
    mixer.setEnable(Gbs::OUT_BOTH3);
}

long WaveVolDemo::setupNextRun(Synth &synth, unsigned counter) {
    if (counter >= N_RUNS) {
        return 0;
    } else {
        WaveChannel &ch3 = synth.getChannels().ch3;
        ch3.setOutputLevel(static_cast<Gbs::WaveVolume>(counter));
        std::cout << "Volume: " << VOL_STRINGS[counter] << std::endl;
        return DEMO_RUNTIME;
    }

}
