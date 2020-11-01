/*
** Trackerboy - Gameboy / Gameboy Color music tracker
** Copyright (C) 2019-2020 stoneface86
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
*/ 

//
// Header file for the audio library.
//
// This library is a utility library for the demo and ui projects.
// Handles querying available devices and buffered audio playback.
//

#pragma once

#include "miniaudio.h"

#include <atomic>
#include <cstdlib>
#include <stdexcept>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#ifdef _MSC_VER
// disable warnings about unscoped enums
#pragma warning(disable : 26812)
#endif

namespace audio {

//
// Container class for all available devices on the system exposed to us
// from a given libsoundio backend. Only output devices that support one or
// more of our supported sample rates are stored here.
//
// It is possible that the table is empty (the host list is empty), if
// this occurs the application should not start as there won't be any
// devices to output sound to.
// 
class DeviceTable {

public:

    struct Device {
        int const deviceId;                 // soundio device index
        int const samplerates;              // bitfield of supported samplerates
    };

    using DeviceVec = std::vector<Device>;
    using DeviceIterator = DeviceVec::const_iterator;

    DeviceTable(ma_context &context);

    // no copying
    DeviceTable(DeviceTable const&) = delete;
    void operator=(DeviceTable const&) = delete;

    Device& operator[](unsigned index);

    //
    // Returns true if there are no available devices, false otherwise.
    //
    bool isEmpty() const noexcept;

    //
    // Get the index of the default device. If there are no devices available,
    // or if the default device is not supported then 0 is returned.
    //
    int defaultDevice() const noexcept;

    int defaultDeviceIndex() const noexcept;

    //
    // Device iterators for the given host api index
    //
    DeviceIterator begin() const noexcept;
    DeviceIterator end() const noexcept;

    //
    // Rescans all available devices and updates the table.
    //
    //void rescan(struct SoundIo *soundio);


private:
    ma_context &mContext;

    DeviceVec mDeviceList;
    int mDefaultDevice;
    int mDefaultDeviceIndex;


};

//
// Container class for all available soundio backends. The table contains a
// SoundIo handle and DeviceTable for each backend. Backends can be accessed
// via index from 0 to size().
//
class BackendTable {


public:

    struct Backend {
        DeviceTable table;

        Backend();
    };

    // Location type is just a pair type with the backend index first
    // followed by device index
    using Location = std::pair<int, int>;

    BackendTable();
    ~BackendTable();

    Backend& operator[](int index);

    //
    // Get a soundio handle to the device at the given location
    //
    struct SoundIoDevice* getDevice(Location location) noexcept;

    //
    // Lookup the device in the given backend with the id. If the device
    // cannot be found, the default one is returned.
    //
    //Location getDeviceLocation(SoundIoBackend backendType, const char *id) noexcept;

    //
    // Lookup the default device. The first connected backend with at least one
    // device is used.
    //
    Location getDefaultDeviceLocation() noexcept;

    //
    // Test if the given backend is connected.
    //
    bool isConnected(int index) const noexcept;

    //
    // Get the name of the backend
    //
    const char* name(int index) const noexcept;

    //
    // Retry connecting to the given backend. 0 is returned on success and the
    // backend's device table is rescanned. On failure, the SoundIoError is
    // returned.
    //
    int reconnect(int index) noexcept;

    //
    // Convenience method. Rescans the backend's DeviceTable
    //
    void rescan(int index) noexcept;

    //
    // Returns the count of available backends.
    //
    unsigned size() const noexcept;

    //
    // Map a SoundIoBackend to an index from 0 <= index < soundio_backend_count
    //
    //int toIndex(SoundIoBackend backend) const noexcept;






private:

    std::unique_ptr<Backend[]> mBackends;
    unsigned mBackendCount;

};



}

