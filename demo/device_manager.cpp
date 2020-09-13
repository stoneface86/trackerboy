
#include "portaudio.h"

#include "audio.hpp"

#include <iostream>
#include <iomanip>


int main(void) {

    PaError err = Pa_Initialize();
    if (err != paNoError) {
        return 1;
    }

    audio::DeviceTable& deviceTable = audio::DeviceTable::instance();

    if (deviceTable.isEmpty()) {
        std::cout << "No available devices :(" << std::endl;
    } else {
        int i = 0;
        for (auto &host : deviceTable.hosts()) {
            std::cout << "Host API: " << host.info->name << std::endl;
            auto devBegin = deviceTable.devicesBegin(i);
            auto devEnd = deviceTable.devicesEnd(i);
            for (auto iter = devBegin; iter != devEnd; ++iter) {
                std::cout << " * " << std::left << std::setw(50) << iter->info->name << " | ";
                if (iter->samplerates & 0x1) {
                    std::cout << "11025 Hz ";
                }
                if (iter->samplerates & 0x2) {
                    std::cout << "22050 Hz ";
                }
                if (iter->samplerates & 0x4) {
                    std::cout << "44100 Hz ";
                }
                if (iter->samplerates & 0x8) {
                    std::cout << "48000 Hz ";
                }
                if (iter->samplerates & 0x10) {
                    std::cout << "96000 Hz ";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
            ++i;
        }
    }

    Pa_Terminate();
    return 0;
}
