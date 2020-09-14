/*
** Header file for the audio library.
**
** This library is a utility library for the demo and ui projects.
** Handles querying available devices and buffered audio playback.
*/

#pragma once

#include <stdexcept>
#include <memory>
#include <vector>

#include "portaudio.h"
#include "pa_ringbuffer.h"

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

extern double const SAMPLERATE_TABLE[SR_COUNT];

//
// Exception class for a portaudio error. The portaudio error code
// that caused this exception can be accessed via the getError() method
//
class PaException : public std::runtime_error {
    PaError err;
public:
    PaException(PaError err);

    PaError getError();
};

//
// Container class for all available devices on the system exposed to us
// via portaudio. Only output devices that support one or more of our
// supported sample rates are stored here.
//
// It is possible that the table is empty (the host list is empty), if
// this occurs the application should not start as there won't be any
// devices to output sound to.
// 
class DeviceTable {

public:
    struct Host {
        int const hostId;                   // Portaudio host api index
        int const deviceOffset;             // starting offset in device vector
        int const deviceCount;              // count of devices this api has
        int const deviceDefault;            // index of the default device
        PaHostApiInfo const * const info;   // info structure
    };

    struct Device {
        int const deviceId;                 // Portaudio device index
        int const samplerates;              // bitfield of supported samplerates
        PaDeviceInfo const * const info;    // info structure
    };

    using HostVec = std::vector<Host>;
    using DeviceVec = std::vector<Device>;
    using HostIterator = HostVec::const_iterator;
    using DeviceIterator = DeviceVec::const_iterator;

    DeviceTable(DeviceTable const&) = delete;
    void operator=(DeviceTable const&) = delete;

    static DeviceTable& instance();

    //
    // Returns true if there are no available devices, false otherwise.
    //
    bool isEmpty() const noexcept;

    //
    // Get the vector of available host apis
    //
    HostVec const& hosts() const noexcept;

    HostIterator hostsBegin() const noexcept;
    HostIterator hostsEnd() const noexcept;

    //
    // Device iterators for the given host api index
    //
    DeviceIterator devicesBegin(int host) const noexcept;
    DeviceIterator devicesEnd(int host) const noexcept;


private:
    DeviceTable();

    HostVec mHostList;
    DeviceVec mDeviceList;


};



//
// Container class keeping track of a user selected host api, device and samplerate
//
class DeviceManager {

public:

    DeviceManager();

    //
    // The current host api in use.
    //
    int currentHost() const noexcept;

    //
    // The current device in use.
    //
    int currentDevice() const noexcept;

    int currentSamplerate() const noexcept;

    //
    // Portaudio device id of the current device
    //
    int portaudioDevice() const noexcept;

    //
    // Return a vector of available sample rates for the current device
    //
    std::vector<Samplerate> const& samplerates() const noexcept;

    //
    // Set the Host Api to use. The default device for the api will be selected.
    //
    void setCurrentApi(int index);
    
    //
    // Set the current device index. This index is relative to the current host api.
    //
    void setCurrentDevice(int index);

    //
    // Set the current device and api from the PaDeviceIndex
    //
    void setPortaudioDevice(int deviceIndex);

    void setCurrentSamplerate(int samplerateIndex);




private:

    DeviceTable &mTable;

    int mCurrentApi;
    int mCurrentDevice;
    int mSamplerateIndex;
    Samplerate mCurrentSamplerate;

    // list of available sample rates for the current device
    std::vector<Samplerate> mSamplerates;

    DeviceTable::HostIterator mCurrentApiIter;
    DeviceTable::DeviceIterator mCurrentDeviceIter;
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


    PlaybackQueue(float samplingRate, unsigned bufferSize = DEFAULT_BUFFER_SIZE);
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
    // Empty the playback queue
    //
    void flush();

    //
    // Change the minimum buffer size of the playback queue. The given size, in
    // milleseconds, should be in the range of MIN_BUFFER_SIZE and
    // MAX_BUFFER_SIZE
    //
    void setBufferSize(unsigned bufferSize);

    //
    // Change the sampling rate for the playback output. The stream must
    // be stopped before calling this method, as a new one will have to be
    // opened. The queue is also flushed.
    //
    void setSamplingRate(float samplingRate);

    //
    // Begin audio playback. Audio data written to the queue is then
    // played out to the speakers. Silence is played out whenever the
    // queue is empty.
    //
    void start();

    //
    // Force stop of the playback stream. If wait is true, then this method
    // will block until all samples in the playback queue are played out.
    // Otherwise, the stream terminates immediately. The queue is also flushed.
    //
    void stop(bool wait);

    //
    // Write the given sample buffer to the playback queue. The stream is then
    // started if the stream was stopped (queue was empty or stop() was called).
    //
    size_t write(float buf[], size_t nsamples);

    //
    // Write the entire sample buffer to the playback queue. This method will
    // block until the entire buffer is written to the queue.
    //
    void writeAll(float buf[], size_t nsamples);

private:

    PaStream *mStream;
    PaUtilRingBuffer mQueue;

    std::vector<float> mQueueData;

    float mSamplingRate;
    unsigned mBufferSize; // size in milleseconds of the buffer
    unsigned mWaitTime; // time in milleseconds to sleep when buffer is full
    

    friend PaStreamCallback playbackCallback;

    //
    // Check if the given bufferSize is valid, throw invalid_argument otherwise
    // A valid buffer size is withing the range of MIN_BUFFER_SIZE and MAX_BUFFER_SIZE, inclusive
    //
    void checkBufferSize(unsigned bufferSize);

    //
    // Check if the given sampling rate is valid, throw invalid_argument otherwise
    // A valid sampling rate is nonzero and positive.
    //
    void checkSamplingRate(float samplingRate);

    //
    // Opens the PaStream stored in mStream
    //
    void openStream();

    void resizeQueue();
};



}

