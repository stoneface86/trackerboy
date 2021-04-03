
#include "interactive.hpp"

#include <iostream>
#include <string>


void InteractiveDemo::audioCallback(ma_device *device, void *out, const void *in, ma_uint32 frames) {
    (void)in;
    auto demo = static_cast<InteractiveDemo*>(device->pUserData);

    if (demo->mLock.test_and_set(std::memory_order_acquire)) {
        return;
    }

    auto &synth = demo->mSynth;
    auto &apu = synth.apu();
    int16_t *out16 = reinterpret_cast<int16_t*>(out);

    while (frames) {

        auto nread = apu.readSamples(out16, frames);
        if (nread == 0) {
            demo->runFrame();
            synth.run();
        } else {
            frames -= nread;
            out16 += nread * 2;
        }

    }

    demo->mLock.clear(std::memory_order_release);
}

InteractiveDemo::InteractiveDemo() :
    mSynth(DEVICE_SAMPLERATE),
    mDevice()
{
}

InteractiveDemo::~InteractiveDemo() {

}

int InteractiveDemo::exec() {

    auto deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = DEVICE_FORMAT;
    deviceConfig.playback.channels = DEVICE_CHANNELS;
    deviceConfig.sampleRate = DEVICE_SAMPLERATE;
    deviceConfig.dataCallback = audioCallback;
    deviceConfig.pUserData = this;

    mDevice.reset(new ma_device);

    if (ma_device_init(NULL, &deviceConfig, mDevice.get()) != MA_SUCCESS) {
        std::cerr << "Could not open playback device." << std::endl;
        return 1;
    }

    if (ma_device_start(mDevice.get()) != MA_SUCCESS) {
        std::cerr << "Could not start playback device." << std::endl;
        return 2;
    }

    auto result = init();
    if (result) {
        return result;
    }

    std::string input;
    do {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input.size() == 0) {
            continue;
        }
    } while (!processLine(input));

    ma_device_uninit(mDevice.get());
    return 0;
}

void InteractiveDemo::lock() {
    while (mLock.test_and_set(std::memory_order_acquire));
}

void InteractiveDemo::unlock() {
    mLock.clear(std::memory_order_release);
}
