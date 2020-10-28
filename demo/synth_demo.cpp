
#include "miniaudio.h"

#include "trackerboy/synth/Synth.hpp"

#include <algorithm>
#include <atomic>
#include <iostream>
#include <string>

using namespace trackerboy;

class Spinlock {

public:

    Spinlock() {
        mFlag.clear();
    }
    ~Spinlock() = default;

    // no copying
    Spinlock(Spinlock const &ref) = delete;
    void operator=(Spinlock const &ref) = delete;


    void lock() noexcept {
        while (mFlag.test_and_set(std::memory_order_acquire));
    }

    bool tryLock() noexcept {
        return !mFlag.test_and_set(std::memory_order_acquire);
    }

    void unlock() noexcept {
        mFlag.clear(std::memory_order_release);
    }

private:

    std::atomic_flag mFlag = ATOMIC_FLAG_INIT;


};

struct AudioContext {

    Synth synth;
    int16_t *buffer;
    int16_t *bufferEnd;
    Spinlock spinlock;

    AudioContext(unsigned samplerate) :
        synth(samplerate),
        buffer(nullptr),
        bufferEnd(nullptr),
        spinlock()
    {
    }
};


void audioCallback(ma_device *device, void *out, const void *in, ma_uint32 frames) {
    (void)in;
    AudioContext *ctx = static_cast<AudioContext*>(device->pUserData);

    auto &synth = ctx->synth;
    auto buffer = ctx->buffer;
    auto bufferEnd = ctx->bufferEnd;
    auto &spinlock = ctx->spinlock;

    int16_t *out16 = reinterpret_cast<int16_t*>(out);

    while (frames) {
        if (buffer == bufferEnd) {
            spinlock.lock();
            size_t frameLen = synth.run();
            buffer = synth.buffer();
            spinlock.unlock();
            bufferEnd = buffer + (frameLen * 2);
        }

        size_t framesToWrite = std::min(static_cast<size_t>(frames), static_cast<size_t>(bufferEnd - buffer) / 2);
        size_t samplesToWrite = framesToWrite * 2;
        std::copy_n(buffer, samplesToWrite, out16);
        buffer += samplesToWrite;
        out16 += samplesToWrite;
        frames -= framesToWrite;
    }

    ctx->buffer = buffer;
    ctx->bufferEnd = bufferEnd;

}


int main() {

    AudioContext ctx(48000);
    ctx.synth.writeRegister(Gbs::REG_NR51, 0x11);
    ctx.synth.writeRegister(Gbs::REG_NR12, 0xF0);
    ctx.synth.writeRegister(Gbs::REG_NR14, 0x80);

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

            // set the frequency
            ctx.spinlock.lock();
            ctx.synth.writeRegister(Gbs::REG_NR13, freq & 0xFF);
            ctx.synth.writeRegister(Gbs::REG_NR14, (freq >> 8) & 0x7);
            ctx.spinlock.unlock();
        }

    }

    ma_device_uninit(device.get());

    return 0;
}
