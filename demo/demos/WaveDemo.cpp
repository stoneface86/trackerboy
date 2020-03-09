
#include <algorithm>
#include <iostream>
#include <sstream>
#include "../demo.hpp"

#define DEMO_RUNTIME 1000
#define N_RUNS 4

using namespace trackerboy;

static const uint16_t FREQ_TABLE[N_RUNS] = {
    1045,  // C3
    1547,  // C4
    1797,  // C5
    1922   // C6
};

const uint8_t WAVEDATA_TRIANGLE[Gbs::WAVE_RAMSIZE] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
    0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
};

const uint8_t WAVEDATA_SAWTOOTH[Gbs::WAVE_RAMSIZE] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
};

const uint8_t WAVEDATA_SINE[Gbs::WAVE_RAMSIZE] = {
    0x8A, 0xBC, 0xEF, 0xFF, 0xFF, 0xFF, 0xEC, 0xBA,
    0x86, 0x54, 0x21, 0x10, 0x00, 0x11, 0x24, 0x56
    //0x8A, 0xBC, 0xDE, 0xFF, 0xFF, 0xFF, 0xFE, 0xCB,
    //0xA8, 0x65, 0x42, 0x11, 0x00, 0x01, 0x12
};



WaveDemo::WaveDemo(const uint8_t wavedata[Gbs::WAVE_RAMSIZE], std::string waveName) :
    wavedata{0},
    Demo("")
{
    std::copy_n(wavedata, Gbs::WAVE_RAMSIZE, this->wavedata);
    std::ostringstream stream;
    stream << "Wave (" << waveName << ")";
    name = stream.str();
}

void WaveDemo::init(Synth &synth) {
    Mixer &mixer = synth.mixer();
    WaveChannel &ch3 = synth.getChannels().ch3;

    ch3.setOutputLevel(Gbs::WAVE_FULL);
    ch3.setWaveform(wavedata);

    mixer.setTerminalEnable(Gbs::TERM_BOTH, true);
    mixer.setEnable(Gbs::OUT_BOTH3);
}

long WaveDemo::setupNextRun(Synth &synth, unsigned counter) {
    if (counter >= N_RUNS) {
        return 0;
    } else {
        uint16_t freq = FREQ_TABLE[counter];
        std::cout << "Frequency: " << freq << std::endl;
        synth.getChannels().ch3.setFrequency(freq);
        return DEMO_RUNTIME;
    }
}
