
#include "trackerboy/export/Wav.hpp"
#include "trackerboy/synth/PulseOsc.hpp"
#include "trackerboy/synth/WaveOsc.hpp"
#include "trackerboy/synth/Synth.hpp"

#include <fstream>

using namespace trackerboy;

static constexpr auto SAMPLING_RATE = 44100u;

void dampen(float buf[], size_t nsamples) {
    for (size_t i = 0; i < nsamples; ++i) {
        buf[i] *= 0.25f;
    }
}


int main() {

    std::ofstream file("test.wav", std::ios::binary | std::ios::out);
    Wav wav(file, 2, SAMPLING_RATE);
    wav.begin();

    Synth synth(SAMPLING_RATE);
    synth.setOutputEnable(Gbs::OUT_BOTH1);

    synth.hardware().osc1.setDuty(Gbs::DUTY_50);
    synth.hardware().osc1.setFrequency(2000);
    synth.hardware().env1.setRegister(0xF0);
    synth.hardware().sweep1.setRegister(0x3A);

    std::vector<float> buf(SAMPLING_RATE * 2);
    auto data = buf.data();
    synth.fill(data, SAMPLING_RATE);
    wav.write(data, SAMPLING_RATE);


    /*PulseOsc osc(SAMPLING_RATE);
    osc.setBufferSize(100);
    osc.setDuty(Gbs::DUTY_50);

    size_t nsamples = 11025;
    std::vector<float> buf(nsamples);
    auto data = buf.data();

    
    osc.generate(data, nsamples);
    dampen(data, nsamples);
    wav.write(data, nsamples);
    osc.setFrequency(1380);
    osc.generate(data, nsamples);
    dampen(data, nsamples);
    wav.write(data, nsamples);
    osc.setFrequency(1000);
    osc.generate(data, nsamples);
    dampen(data, nsamples);
    wav.write(data, nsamples);*/

    /*WaveOsc osc(SAMPLING_RATE);
    Waveform wave;
    auto *wavedata = wave.data();
    wavedata[0] = 0x01;
    wavedata[1] = 0x23;
    wavedata[2] = 0x45;
    wavedata[3] = 0x67;
    wavedata[4] = 0x89;
    wavedata[5] = 0xAB;
    wavedata[6] = 0xCD;
    wavedata[7] = 0xEF;
    wavedata[8] = 0xFE;
    wavedata[9] = 0xDC;
    wavedata[10] = 0xBA;
    wavedata[11] = 0x98;
    wavedata[12] = 0x76;
    wavedata[13] = 0x54;
    wavedata[14] = 0x32;
    wavedata[15] = 0x10;
    osc.setWaveform(wave);


    osc.setBufferSize(0);
    std::vector<float> period;

    for (uint16_t i = 0; i != 2048; ++i) {
        osc.setFrequency(i);
        osc.copyPeriod(period);
        size_t nsamples = period.size();
        float *data = period.data();
        for (size_t i = 0; i != nsamples; ++i) {
            data[i] *= 0.25f;
        }

        wav.write(data, nsamples);
    }*/

    //osc.setFrequency(1923);
    /*osc.copyPeriod(period);

    float *data = period.data();
    size_t nsamples = period.size();

    
    osc.generate(data, nsamples);
    
    for (size_t i = 0; i != nsamples; ++i) {
        data[i] *= 0.25f;
    }

    wav.write(data, nsamples);*/
    wav.finish();

    file.close();



    return 0;
}
