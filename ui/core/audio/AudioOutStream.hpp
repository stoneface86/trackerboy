
#pragma once

#include "core/audio/Ringbuffer.hpp"

#include "miniaudio.h"

#include <QMutex>
#include <QWaitCondition>

#include <atomic>
#include <chrono>
#include <cstddef>


class AudioOutStream {

public:

    AudioOutStream();

    //
    // Writer for the stream's ringbuffer. Write audio data to this buffer
    // when the stream is playing.
    //
    AudioRingbuffer::Writer buffer();

    // diagnostics

    //
    // size, in samples, of the stream's buffer.
    //
    size_t bufferSize();

    //
    // Gets the total count of underruns that have occurred. Does not reset
    // on stream start.
    //
    unsigned underruns();

    //
    // Gets the total count of samples written to the device. Count is reset
    // on stream start.
    //
    unsigned elapsed();

    long long syncTime();

    //
    // Resets the underrun counter
    //
    void resetUnderruns();

    // true when the queue has finished playing out
    bool finished();

    //
    // If a thread is waiting for a sync it is immediately woken up, otherwise
    // this function does nothing. Typically used for abruptly stopping a stream.
    //
    void interrupt();

    //
    // Sets the miniaudio device to use when streaming. 
    //
    void setDevice(ma_device *device);

    //
    // Begin device playback. The result from ma_device_start is returned. If an error
    // ocurred, the buffer is reset.
    //
    ma_result start();

    //
    // Stop device playback. The buffer is reset if the device was sucessfully stopped.
    //
    void stop();

    //
    // Sets the idle status of the stream. An idle stream will not count underruns
    // when they occur. When you are no longer adding data to the buffer, set this to true.
    //
    void setIdle(bool idle);

    //
    // Synchronize with the callback, blocks until the callback has written an
    // entire period to the output device.
    //
    void sync();

    //
    // Callback function for the output stream. Set ma_device_config::dataCallback to this
    // when initializing a device.
    //
    static void callback(ma_device *device, void *out, const void *in, ma_uint32 frames);

private:

    using Clock = std::chrono::steady_clock;

    void initCallback(ma_device *device);

    void handleCallback(int16_t *out, size_t frames);

    ma_device *mDevice;

    // max time to wait when syncing, in milliseconds
    unsigned mSyncTimeout;

    // atomics since they are modified by the callback
    std::atomic_bool mSync;
    // stream is idle or is starting/stopping, do not count underruns when they occur
    std::atomic_bool mIdling;
    std::atomic_uint mUnderruns;
    std::atomic_uint mSamplesElapsed;
    
    QMutex mMutex;
    QWaitCondition mSyncCondition;
    
    AudioRingbuffer mBuffer;
    size_t mSyncCounter;
    size_t mSyncPeriod;

    Clock::time_point mLastSyncTime;
    Clock::duration mLatency;

};
