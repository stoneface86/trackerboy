
#include "audio.hpp"

#include <cstring>

namespace audio {


BackendTable::Backend::Backend() :
    soundio(nullptr),
    backendType(SoundIoBackendNone),
    table()
{
}


BackendTable::BackendTable() {

    // we only need this struct for getting the backend count
    struct SoundIo *soundio = soundio_create();
    mBackendCount = soundio_backend_count(soundio);

    mBackends.reset(new Backend[mBackendCount]);

    for (unsigned i = 0; i < mBackendCount; ++i) {
        struct SoundIo *backend = soundio_create();
        auto backendType = soundio_get_backend(soundio, i);
        int err = soundio_connect_backend(backend, backendType);
        auto entry = mBackends.get() + i;
        entry->soundio = backend;
        entry->backendType = backendType;
        if (err == 0) {
            entry->table.rescan(backend);
        }

    }

    soundio_destroy(soundio);
}

BackendTable::~BackendTable() {
    for (unsigned i = 0; i != mBackendCount; ++i) {
        soundio_destroy(mBackends[i].soundio);
    }
}

BackendTable::Backend& BackendTable::operator[](int index) {
    return mBackends[index];
}

struct SoundIoDevice* BackendTable::getDevice(Location location) noexcept {
    auto &backend = mBackends[location.first];
    return soundio_get_output_device(
                backend.soundio,
                backend.table[location.second].deviceId
            );
}

BackendTable::Location BackendTable::getDeviceLocation(SoundIoBackend backendType, const char *id) noexcept {

    int index = toIndex(backendType);
    if (index == -1) {
        // failed, unsupported backend
        return getDefaultDeviceLocation();
    }

    auto &backend = mBackends[index];
    if (backend.soundio->current_backend == SoundIoBackendNone) {
        // failed, backend not connected
        return getDefaultDeviceLocation();
    }

    int devIndex = 0;
    for (auto dev : backend.table) {
        struct SoundIoDevice *device = soundio_get_output_device(backend.soundio, dev.deviceId);
        if (device != nullptr) {
            if (strcmp(id, device->id) == 0) {
                // success, return the device
                return Location(index, devIndex);
            }
        }
        soundio_device_unref(device);
        ++devIndex;
    }

    // failed, could not find device with given id
    return getDefaultDeviceLocation();


}

BackendTable::Location BackendTable::getDefaultDeviceLocation() noexcept {
    Backend* backendIter = mBackends.get();
    for (unsigned i = 0; i != mBackendCount; ++i) {
        auto soundio = backendIter->soundio;
        auto &table = backendIter->table;
        if (soundio->current_backend != SoundIoBackendNone) {
            if (!table.isEmpty()) {
                return Location(i, table.defaultDeviceIndex());
            }
        }
        ++backendIter;
    }
    return Location(-1, -1); // this SHOULD never happen, worst case the dummy device gets chosen
}


bool BackendTable::isConnected(int index) const noexcept {
    return mBackends[index].soundio->current_backend != SoundIoBackendNone;
}

const char* BackendTable::name(int index) const noexcept {
    return soundio_backend_name(mBackends[index].backendType);
}

int BackendTable::reconnect(int index) noexcept {
    auto &backend = mBackends[index];
    return soundio_connect_backend(backend.soundio, backend.backendType);
}

void BackendTable::rescan(int index) noexcept {
    auto &backend = mBackends[index];
    backend.table.rescan(backend.soundio);
}

unsigned BackendTable::size() const noexcept {
    return mBackendCount;
}

int BackendTable::toIndex(SoundIoBackend backend) const noexcept {
    Backend *backendIter = mBackends.get();
    for (unsigned i = 0; i != mBackendCount; ++i) {
        if (backendIter->backendType == backend) {
            return i;
        }
        ++backendIter;
    }
    return -1;
}

}
