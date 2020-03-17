
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
    WaveOsc &osc3 = synth.hardware().osc3;

    Waveform wave;
    std::copy_n(WAVEDATA_TRIANGLE, Gbs::WAVE_RAMSIZE, wave.data());
    osc3.setWaveform(wave);
    osc3.setFrequency(1045);

    synth.setOutputEnable(Gbs::OUT_BOTH3);
}

long WaveVolDemo::setupNextRun(Synth &synth, unsigned counter) {
    if (counter >= N_RUNS) {
        return 0;
    } else {
        //WaveChannel &ch3 = synth.getChannels().ch3;
        //ch3.setOutputLevel(static_cast<Gbs::WaveVolume>(counter));
        std::cout << "Volume: " << VOL_STRINGS[counter] << std::endl;
        return DEMO_RUNTIME;
    }

}
