
#pragma once

#include "core/Config.hpp"
#include "core/audio/Ringbuffer.hpp"
#include "core/Guarded.hpp"

#include "miniaudio.h"
#include "rtaudio/RtAudio.h"

#include <QMutex>
#include <QObject>

#include <atomic>
#include <cstdint>

//
// AudioStream class. Manages an RtAudio device and a playback buffer for
// asynchronous sound output.
// 
// All functions in this class are thread-safe
//
class AudioStream {

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
    // Gets the last RtAudio error that occurred. RtAudioError::UNSPECIFIED is
    // returned if no error has occured.
    //
    RtAudioError::Type lastError();
    
    //
    // Applies the configuration and opens a stream for the configured device.
    // On success the stream is enabled, and audio can now be played out. On
    // failure the stream is disabled, even if the previous configuration
    // worked. If the stream was running when this function is called, it is
    // stopped and then restarted with the new configuration.
    //
    void setConfig(Config::Sound const& config);

    //
    // Gets the current count of underruns reported by RtAudio.
    //
    int underruns() const;

    //
    // Gets the current utilization of the buffer, in samples. 
    //
    int bufferUsage() const;

    //
    // Gets the size of the buffer, in samples.
    //
    size_t bufferSize();

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

private:

    // NOTE: functions prefixed with an underscore do not lock the mutex

    bool _isEnabled();

    bool _isRunning();

    //
    // Closes the stream if it is open, does nothing otherwise.
    //
    void _close();

    //
    // Disables the stream. If a stream is open it is closed, and then the
    // pointer to RtAudio handle is set to null.
    //
    void _disable();

    //
    // error handler for RtAudioError. Logs a specified message and RtAudio's
    // message via qCritical. Stores the error's type in mLastError and then disables
    // the stream.
    //
    void handleError(const char *msg, RtAudioError const& err);

    //
    // Starts the stream for the given RtAudio handle
    //
    void _start(Guarded<RtAudio> *api);
    
    static int audioCallback(
        void *output,
        void *input,
        unsigned bufferFrames,
        double streamTime,
        RtAudioStreamStatus status,
        void *userData
    );

    int handleAudio(int16_t *output, size_t bufferFrames, double streamTime, RtAudioStreamStatus status);

    QMutex mMutex;

    Guarded<RtAudio> *mApi;
    RtAudioError::Type mLastError;
    AudioRingbuffer mBuffer;

    // current stream settings (modified by setConfig)
    unsigned mSamplerate;

    size_t mPlaybackDelay;

    std::atomic_int mUnderruns;
    std::atomic_int mBufferUsage;
};
