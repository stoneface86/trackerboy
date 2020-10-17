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

#include "soundio/soundio.h"

#include <atomic>
#include <cstdlib>
#include <stdexcept>
#include <memory>
#include <utility>
#include <vector>

#ifdef _MSC_VER
// disable warnings about unscoped enums
#pragma warning(disable : 26812)
#endif

namespace audio {


//
// Enum of sample rates for audio output. Since devices only support a limited
// range or specific rates we will limit the user to this selection. Output is
// bandlimited so using a higher sampling rate to avoid aliasing is unnecessary.
//
enum Samplerate {
    SR_11025,           // Bit 0, 11,025 Hz
    SR_22050,           // Bit 1, 22,050 Hz
    SR_44100,           // Bit 2, 44,100 Hz
    SR_48000,           // Bit 3, 48,000 Hz
    SR_96000,           // Bit 4, 96,000 Hz

    SR_COUNT
};

extern unsigned const SAMPLERATE_TABLE[SR_COUNT];


class SoundIoError : public std::runtime_error {
    int mError;
public:
    SoundIoError(int error) noexcept;

    int error() const noexcept;
};

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

    DeviceTable();

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
    void rescan(struct SoundIo *soundio);


private:
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
        struct SoundIo * soundio;
        SoundIoBackend backendType;
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
    Location getDeviceLocation(SoundIoBackend backendType, const char *id) noexcept;

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
    int toIndex(SoundIoBackend backend) const noexcept;






private:

    std::unique_ptr<Backend[]> mBackends;
    unsigned mBackendCount;

};

//
// Class for an audio playback queue. Samples to be played out are stored in
// the queue by calling write or writeAll. The samples will be played out
// to the default device when the stream is started.
//
class PlaybackQueue {

public:

    // maximum buffer size of 500 milleseconds
    static constexpr unsigned MAX_BUFFER_SIZE = 500;
    static constexpr unsigned MIN_BUFFER_SIZE = 1;
    static constexpr unsigned DEFAULT_BUFFER_SIZE = 40;


    PlaybackQueue(Samplerate samplerate, unsigned bufferSize = DEFAULT_BUFFER_SIZE);
    ~PlaybackQueue();

    //
    // Size of the playback buffer, by number of samples.
    //
    size_t bufferSampleSize();

    //
    // Minimum size of the playback queue, in milleseconds.
    //
    unsigned bufferSize();

    //
    // Check if a write can be made to the queue for the given number
    // of samples.
    //
    bool canWrite(size_t nsamples);

    //
    // Closes the portaudio stream. The stream must be stopped before
    // calling this method.
    //
    void close();

    //
    // Opens an output stream using the set device. Samples written to the
    // queue will be played as soon as the queue is completely filled. The
    // stream will stop playing when the queue is emptied or when stop() is
    // called.
    //
    void open();

    //
    // Change the minimum buffer size of the playback queue. The given size, in
    // milleseconds, should be in the range of MIN_BUFFER_SIZE and
    // MAX_BUFFER_SIZE
    //
    void setBufferSize(unsigned bufferSize);

    //
    // Set the output device and samplerate. In order for changes to take effect
    // you must close the current stream and open a new one.
    //
    void setDevice(struct SoundIoDevice *device, Samplerate samplerate);

    //
    // Starts the playback stream. Stream will underrun if there is not enough
    // samples in the queue.
    //
    void start();

    //
    // Force stop of the playback stream. If wait is true, then this method
    // will block until all samples in the playback queue are played out.
    // Otherwise, the stream terminates immediately. The queue is also flushed.
    //
    void stop(bool wait);

    //
    // Write the given sample buffer to the playback queue. If this write will
    // completely fill the buffer and the stream is inactive, then the stream
    // will be started.
    //
    size_t write(int16_t buf[], size_t nsamples);

    //
    // Write the entire sample buffer to the playback queue. This method will
    // block until the entire buffer is written to the queue.
    //
    void writeAll(int16_t buf[], size_t nsamples);

    //
    // Get the total count of buffer underruns since last reset.
    //
    unsigned underflows() const noexcept;

    //
    // Set the underflow counter to 0.
    //
    void resetUnderflows() noexcept;

private:

    enum class State {
        stopped,
        running,
        paused
    };

    static void playbackCallback(struct SoundIoOutStream *stream, int framesMin, int framesMax);
    static void underflowCallback(struct SoundIoOutStream *stream);

    struct SoundIoDevice *mDevice;

    // the output stream
    struct SoundIoOutStream *mStream;

    // sample data to play out
    struct SoundIoRingBuffer *mRingbuffer;

    Samplerate mSamplerate;
    unsigned mBufferSize; // size in milleseconds of the buffer
    bool mResizeRequired;

    unsigned mWaitTime;

    std::atomic_bool mStopping;
    std::atomic_uint mUnderflowCounter;
    State mState;




    //
    // Check if the given bufferSize is valid, throw invalid_argument otherwise
    // A valid buffer size is withing the range of MIN_BUFFER_SIZE and MAX_BUFFER_SIZE, inclusive
    //
    void checkBufferSize(unsigned bufferSize);

    void openStream();

};



}

