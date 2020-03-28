
#include <algorithm>
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
    WaveGen &gen3 = synth.hardware().gen3;

    Waveform wave;
    std::copy_n(WAVEDATA_TRIANGLE, Gbs::WAVE_RAMSIZE, wave.data());
    gen3.copyWave(wave);
    gen3.setFrequency(1045);

    synth.setOutputEnable(Gbs::OUT_BOTH3);
}

long WaveVolDemo::setupNextRun(Synth &synth, unsigned counter) {
    if (counter >= N_RUNS) {
        return 0;
    } else {
        synth.hardware().gen3.setVolume(static_cast<Gbs::WaveVolume>(counter));
        std::cout << "Volume: " << VOL_STRINGS[counter] << std::endl;
        return DEMO_RUNTIME;
    }

}
