#pragma once

#include "audio/AudioEnumerator.hpp"
#include "audio/Ringbuffer.hpp"

#include "miniaudio.h"

#include <QObject>

#include <atomic>
#include <cstddef>

//
// AudioStream class. Manages a miniaudio device and a playback buffer for
// asynchronous sound output.
//
class AudioStream : public QObject {

    Q_OBJECT

public:
    explicit AudioStream(QObject *parent = nullptr);

    //
    // Determines if the stream is enabled. If enabled, audio can be played
    // out by starting the stream and then writing to its buffer. The stream
    // is disabled if open() fails or if a device error occurs when starting
    // or stopping the stream. The only way to enable the stream is by calling
    // open().
    //
    bool isEnabled() const;

    //
    // Determines if the stream is running. Always returns false if the stream
    // is disabled.
    //
    bool isRunning() const;

    //
    // Gets the total count of underruns that have occurred. An underrun occurs
    // when there is not enough samples in the buffer when requested by the
    // device.
    //
    unsigned underruns() const;

    //
    // Gets the size of the buffer, in samples. The size of the buffer is determined
    // by the latency parameter in open().
    //
    size_t bufferSize() const;

    void setDraining(bool draining);

    //
    // Resets the underrun counter to 0.
    //
    void resetUnderruns();

    //
    // Opens an output stream for the configured device.
    // On success the stream is enabled, and audio can now be played out. On
    // failure the stream is disabled. If the stream was running when this
    // function is called, it is stopped and then restarted.
    //
    // NOTE: this function should only be called from the GUI thread
    //
    void open(AudioEnumerator::Device const& device, int samplerate, int latency);

    AudioRingbuffer::Writer writer();

    bool start();

    bool stop();

    void disable();

signals:

    void aborted();

private:

    static void deviceDataCallback(ma_device *device, void *out, const void *in, ma_uint32 frames);
    void handleData(float *out, size_t frames);

    static void deviceStopCallback(ma_device *device);
    void handleStop();

    void handleError(const char *msg, ma_result err);

    //
    // Wrapper for a ma_device, ensures that the wrapped device is uninit'd on
    // destruction.
    //
    class MaDeviceWrapper {

    public:
        MaDeviceWrapper();
        ~MaDeviceWrapper();

        ma_result init(ma_context *ctx, ma_device_config const* config);

        void uninit();

        ma_device* get();

    private:
        bool mInitialized;
        ma_device mDevice;
    };

    bool mEnabled;
    std::atomic_bool mRunning;
    AudioRingbuffer mBuffer;

    MaDeviceWrapper mDevice;
    size_t mPlaybackDelay;

    std::atomic_uint mUnderruns;
    std::atomic_bool mDraining;

};

