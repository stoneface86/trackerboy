

#include "audio.hpp"

#include <iostream>
#include <iomanip>

void printDevices(struct SoundIo *soundio, audio::DeviceTable &deviceTable) {
    if (deviceTable.isEmpty()) {
        std::cout << "No available devices :(" << std::endl;
    } else {
        std::cout << "Default device: " << deviceTable.defaultDevice() << std::endl;

        auto devBegin = deviceTable.begin();
        auto devEnd = deviceTable.end();
        for (auto iter = devBegin; iter != devEnd; ++iter) {
            auto device = soundio_get_output_device(soundio, iter->deviceId);
            std::cout << " " << iter->deviceId << ": " << std::left << std::setw(50) << device->name << " | ";
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
            soundio_device_unref(device);
        }
        std::cout << std::endl;
    }
}


int main(void) {

    audio::BackendTable backendTable;

    unsigned const backendCount = backendTable.size();

    std::cout << "Backends:";
    for (unsigned i = 0; i != backendCount; ++i) {
        std::cout << " " << backendTable.name(i);
    }
    std::cout << std::endl << std::endl;


    for (unsigned i = 0; i != backendCount; ++i) {
        std::cout << backendTable.name(i) << " | ";
        if (backendTable.isConnected(i)) {
            auto &backend = backendTable[i];
            printDevices(backend.soundio, backend.table);
        } else {
            std::cout << "Disconnected" << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}
