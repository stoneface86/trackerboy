/*
* Volume demo
* 
* Tests volume scaling starting at 0% and ending at 100% in 10% intervals.
* All channels are playing at maximum volume.
* Demo is also used for ensuring no clamping occurs at max volume, if
* clamping does occur, Synth::HEADROOM needs to be decreased in volume
*/

#include "miniaudio.h"
#include "trackerboy/Synth.hpp"
#include "trackerboy/data/Waveform.hpp"

using namespace trackerboy;

constexpr unsigned SAMPLERATE = 48000;

int main() {

    Synth synth(SAMPLERATE);
    auto &apu = synth.apu();

    ma_encoder_config config = ma_encoder_config_init(ma_resource_format_wav, ma_format_s16, 2, SAMPLERATE);
    ma_encoder encoder;
    auto err = ma_encoder_init_file("volume_demo.wav", &config, &encoder);
    if (err != MA_SUCCESS) {
        return 1;
    }

    Waveform triangle;
    triangle.fromString("0123456789ABCDEFFEDCBA9876543210");
    auto wavedata = triangle.data();
    
    
    unsigned volume = 100;
    for (unsigned volume = 0; volume <= 100; volume += 10) {
        synth.setVolume(volume);
        synth.reset();

        // copy wave
        for (int i = 0; i != 16; ++i) {
            apu.writeRegister(gbapu::Apu::REG_WAVERAM + i, wavedata[i]);
        }
        // DAC ON
        apu.writeRegister(gbapu::Apu::REG_NR30, 0x80);
        
        // set volumes to max
        apu.writeRegister(gbapu::Apu::REG_NR12, 0xF0);
        apu.writeRegister(gbapu::Apu::REG_NR22, 0xF0);
        apu.writeRegister(gbapu::Apu::REG_NR32, 0x20);
        apu.writeRegister(gbapu::Apu::REG_NR42, 0xF0);
        // set noise
        apu.writeRegister(gbapu::Apu::REG_NR43, 0x64);

        // output to all terminals
        apu.writeRegister(gbapu::Apu::REG_NR51, 0xFF);
        
        // retrigger
        apu.writeRegister(gbapu::Apu::REG_NR14, 0x87);
        apu.writeRegister(gbapu::Apu::REG_NR24, 0x87);
        apu.writeRegister(gbapu::Apu::REG_NR34, 0x84);
        apu.writeRegister(gbapu::Apu::REG_NR44, 0x80);

        for (int i = 0; i != 30; ++i) {
            auto samples = synth.run();
            ma_encoder_write_pcm_frames(&encoder, synth.buffer(), samples);
        }
    }


    ma_encoder_uninit(&encoder);

    return 0;
}
