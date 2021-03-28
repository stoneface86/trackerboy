
#include "miniaudio.h"

#include "trackerboy/Synth.hpp"

#include <algorithm>
#include <atomic>
#include <iostream>
#include <string>

using namespace trackerboy;


struct AudioContext {

    Synth synth;
    std::atomic_flag changeFrequency;
    std::atomic_uint16_t frequency;

    AudioContext(unsigned samplerate) :
        synth(samplerate)
    {
    }
};


void audioCallback(ma_device *device, void *out, const void *in, ma_uint32 frames) {
    (void)in;
    AudioContext *ctx = static_cast<AudioContext*>(device->pUserData);

    auto &synth = ctx->synth;
    auto &apu = synth.apu();

    if (!ctx->changeFrequency.test_and_set()) {
        auto freq = ctx->frequency.load();
        apu.writeRegister(gbapu::Apu::REG_NR13, freq & 0xFF);
        apu.writeRegister(gbapu::Apu::REG_NR14, (freq >> 8) & 0x7);
    }

    int16_t *out16 = reinterpret_cast<int16_t*>(out);

    while (frames) {

        auto nread = apu.readSamples(out16, frames);
        if (nread == 0) {
            synth.run();
        } else {
            frames -= nread;
            out16 += nread * 2;
        }

    }

}


int main() {

    AudioContext ctx(48000);

    auto &apu = ctx.synth.apu();
    apu.writeRegister(gbapu::Apu::REG_NR52, 0x80);
    apu.writeRegister(gbapu::Apu::REG_NR50, 0x77);
    apu.writeRegister(gbapu::Apu::REG_NR51, 0x11);
    apu.writeRegister(gbapu::Apu::REG_NR12, 0xF0);
    apu.writeRegister(gbapu::Apu::REG_NR14, 0x80);

    auto deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_s16;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate = 48000;
    deviceConfig.dataCallback = audioCallback;
    deviceConfig.pUserData = &ctx;

    std::unique_ptr<ma_device> device(new ma_device{ 0 });

    if (ma_device_init(NULL, &deviceConfig, device.get()) != MA_SUCCESS) {
        std::cerr << "Could not open playback device." << std::endl;
        return 1;
    }

    if (ma_device_start(device.get()) != MA_SUCCESS) {
        std::cerr << "Could not start playback device." << std::endl;
        return 2;
    }

    std::cout << "Playing CH1. Enter a number to set frequency (0-2047) or q to quit" << std::endl;
    
    for (;;) {
        std::string input;
        std::getline(std::cin, input);

        if (input.size() == 0) {
            continue;
        }

        if (input[0] == 'q') {
            break;
        } else {
            uint16_t freq;
            try {
                freq = std::stoul(input, nullptr, 10);
            } catch (std::invalid_argument e) {
                std::cout << "Invalid number" << std::endl;
                continue;
            } catch (std::out_of_range e) {
                std::cout << "Frequency too large" << std::endl;
                continue;
            }

            if (freq > GB_MAX_FREQUENCY) {
                std::cout << "Frequency too large" << std::endl;
                continue;
            }

            // set the frequency
            ctx.frequency = freq;
            ctx.changeFrequency.clear();
        }

    }

    ma_device_uninit(device.get());

    return 0;
}
