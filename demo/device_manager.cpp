
#include "portaudio.h"

#include "audio.hpp"

#include <iostream>


int main(void) {

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        return 1;
    }

    int hosts = Pa_GetHostApiCount();
    std::cout << "Host APIs: " << hosts << std::endl;
    for (int i = 0; i != hosts; ++i) {
        auto info = Pa_GetHostApiInfo(i);
        std::cout << "Name: " << info->name;
        std::cout << " Devices: " << info->deviceCount << std::endl;
    }

    audio::DeviceManager manager;

    auto &devices = manager.devices();

    std::cout << "Total devices: " << devices.size() << std::endl;

    for (auto &device : manager.devices()) {
        std::cout << "Name: '" << device.mInfo->name << "' ";
        std::cout << "Sampling rates: ";

        if (device.mSamplerates & 0x1) {
            std::cout << "11025 Hz ";
        }
        if (device.mSamplerates & 0x2) {
            std::cout << "22050 Hz ";
        }
        if (device.mSamplerates & 0x4) {
            std::cout << "44100 Hz ";
        }
        if (device.mSamplerates & 0x8) {
            std::cout << "48000 Hz ";
        }
        if (device.mSamplerates & 0x10) {
            std::cout << "96000 Hz ";
        }
        std::cout << std::endl;
    }

    Pa_Terminate();
    return 0;
}
