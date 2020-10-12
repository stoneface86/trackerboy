

#include "audio.hpp"

#include <iostream>
#include <iomanip>

void printDevices(struct SoundIo *soundio, audio::DeviceTable &deviceTable) {
    if (deviceTable.isEmpty()) {
        std::cout << "No available devices :(" << std::endl;
    } else {
        std::cout << "Backend: " << soundio_backend_name(soundio->current_backend);
        std::cout << " | Default device: " << deviceTable.defaultDevice() << std::endl;

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

    auto soundio = soundio_create();
    if (soundio == nullptr) {
        std::cerr << "out of memory" << std::endl;
    }

    int backendCount = soundio_backend_count(soundio);
    std::cout << "Backends:";
    for (int i = 0; i != backendCount; ++i) {
        std::cout << " " << soundio_backend_name(soundio_get_backend(soundio, i));
    }
    std::cout << std::endl << std::endl;

    audio::DeviceTable deviceTable;
    int err;
    for (int i = 0; i != backendCount; ++i) {
        auto backend = soundio_get_backend(soundio, i);
        err = soundio_connect_backend(soundio, backend);
        if (err) {
            std::cerr << "Couldn't connect to backend " << soundio_backend_name(backend);
            std::cerr << ": " << soundio_strerror(err) << std::endl << std::endl;
            continue;
        }
        deviceTable.rescan(soundio);
        printDevices(soundio, deviceTable);

        soundio_disconnect(soundio);
    }


    soundio_destroy(soundio);
    return 0;
}
