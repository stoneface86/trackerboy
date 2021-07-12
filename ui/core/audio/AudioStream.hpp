
#pragma once

#include "core/Config.hpp"
#include "core/audio/Ringbuffer.hpp"

#include "miniaudio.h"

#include <QObject>

#include <atomic>
#include <cstdint>

//
// AudioStream class. Manages a miniaudio device and a playback buffer for
// asynchronous sound output.
//
class AudioStream : public QObject {

    Q_OBJECT

public:

    explicit AudioStream();
    ~AudioStream();

    //
    // Determines if the stream is enabled. If enabled, audio can be played
    // out by starting the stream and then writing to its buffer. The stream
    // is disabled if setConfig fails or if a device error occurs when starting
    // or stopping the stream. The only way to enable the stream is by calling
    // setConfig.
    //
    bool isEnabled();

    //
    // Determines if the stream is running. Always returns false if the stream
    // is disabled.
    //
    bool isRunning();
    
    //
    // Applies the configuration and opens a stream for the configured device.
    // On success the stream is enabled, and audio can now be played out. On
    // failure the stream is disabled, even if the previous configuration
    // worked. If the stream was running when this function is called, it is
    // stopped and then restarted with the new configuration.
    //
    // NOTE: this function should only be called from the GUI thread
    //
    void setConfig(Config::Sound const& config);

    //
    // Currently unused
    //
    int underruns() const;

    //
    // Resets the underrun counter to 0. 
    //
    void resetUnderruns();

    //
    // Gets the size of the buffer, in samples.
    //
    size_t bufferSize();

    //
    // Unused, always returns 0.0
    //
    double elapsed();

    //
    // Get the buffer writer. If the stream is playing, samples written to
    // this buffer will be played out to the configured device. Only one
    // thread may use this writer at a time.
    //
    AudioRingbuffer::Writer writer();

    //
    // start the stream, true is returned on success. If a stream is already
    // running this function does nothing.
    //
    bool start();

    //
    // stop the stream, true is returned on success. If the stream isn't
    // running, this function does nothing.
    //
    bool stop();

    //
    // Disable the stream.
    //
    void disable();

signals:

    //
    // Emitted if the stream was aborted due to device error
    //
    void aborted();

private:

    void handleError(const char *msg, ma_result err);

    static void deviceDataCallback(ma_device *device, void *out, const void *in, ma_uint32 frames);
    void handleData(int16_t *out, size_t frames);

    static void deviceStopCallback(ma_device *device);
    void handleStop();


    bool mEnabled;
    std::unique_ptr<ma_device> mDevice; // heap alloc because sizeof(ma_device) is 22448!

    AudioRingbuffer mBuffer;

    // current stream settings (modified by setConfig)
    unsigned mSamplerate;

    std::atomic_bool mRunning;

    size_t mPlaybackDelay;

    std::atomic_int mUnderruns;
};
