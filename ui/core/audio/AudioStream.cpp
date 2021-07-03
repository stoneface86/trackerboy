
#include "core/audio/AudioStream.hpp"
#include "core/audio/AudioProber.hpp"
#include "core/samplerates.hpp"

#include <QMutexLocker>
#include <QtDebug>

#include <algorithm>

static const char* LOG_PREFIX = "[AudioStream]";

static void errorCallback(RtAudioError::Type type, std::string const& message) {
    Q_UNUSED(type)

    // only thing we can do here is just log the message, we don't have a void
    // pointer so we can't disable the AudioStream instance that is responsible
    // for this error. (We could make AudioStream a singleton but eh)
    qCritical().noquote() << LOG_PREFIX << QString::fromStdString(message);
}



AudioStream::AudioStream() :
    mMutex(),
    mApi(nullptr),
    mLastError(RtAudioError::UNSPECIFIED),
    mBuffer(),
    mSamplerate(0),
    mPlaybackDelay(0),
    mUnderruns(0)
{
}

AudioStream::~AudioStream() {
    QMutexLocker locker(&mMutex);
    _close();
}


bool AudioStream::isEnabled() {
    QMutexLocker locker(&mMutex);
    return _isEnabled();
}

bool AudioStream::_isEnabled() {
    return mApi != nullptr;
}

bool AudioStream::isRunning() {
    QMutexLocker locker(&mMutex);
    return _isRunning();
}

bool AudioStream::_isRunning() {
    if (_isEnabled()) {
        auto handle = mApi->access();
        return handle->isStreamRunning();
    } else {
        return false;
    }
}

RtAudioError::Type AudioStream::lastError() {
    QMutexLocker locker(&mMutex);
    return mLastError;
}

int AudioStream::underruns() const {
    // mUnderruns is atomic, mutex not needed
    return mUnderruns.load();
}

void AudioStream::resetUnderruns() {
    mUnderruns = 0;
}

size_t AudioStream::bufferSize() {
    QMutexLocker locker(&mMutex);
    return mBuffer.size();
}

double AudioStream::elapsed() {
    QMutexLocker locker(&mMutex);
    if (_isEnabled()) {
        return mApi->access()->getStreamTime();
    } else {
        return 0.0;
    }
}

AudioRingbuffer::Writer AudioStream::writer() {
    QMutexLocker locker(&mMutex);
    return mBuffer.writer();
}

void AudioStream::setConfig(Config::Sound const& config) {

    // get the current running state
    // if we are running then we will have to start the newly opened stream
    bool running = isRunning();

    auto const samplerate = SAMPLERATE_TABLE[config.samplerateIndex];
    
    auto &prober = AudioProber::instance();
    // get the configured api
    auto *api = prober.backend(config.backendIndex);
    if (api == nullptr) {
        // no API found! (this should rarely happen, only possible if RtAudio has no available APIs to use)
        qCritical() << LOG_PREFIX << "setConfig: the configured API is not available";
        QMutexLocker locker(&mMutex);
        _disable();
        return;
    }

    // now we must find the configured device, and attempt to open a stream for it
    // probe devices again, (in case any devices were connected/disconnected)
    prober.probe(config.backendIndex);
    auto deviceIndex = prober.findDevice(config.backendIndex, config.deviceName);
    if (deviceIndex == -1) {
        // device not found!
        qCritical() << LOG_PREFIX << "setConfig: the configured device was not found";
        QMutexLocker locker(&mMutex);
        _disable();
        return;
    }
    auto rtaudioDevice = prober.mapDeviceIndex(config.backendIndex, deviceIndex);

    // stop and close any current streams
    mMutex.lock();
    if (mApi) {
        auto handle = mApi->access();
        if (handle->isStreamRunning()) {
            handle->stopStream();
            handle->closeStream();
        } else if (handle->isStreamOpen()) {
            handle->closeStream();
        }
    }

    mApi = api;

    // apply the configuration

    // initialize the internal buffer

    mBuffer.init((size_t)(config.latency * samplerate / 1000));
    mMutex.unlock();

    // for the rtaudio buffer size, we want this to be as small as possible, since
    // we are using our own buffer. This means the callback will be called much more often,
    // but since the buffer is lock-free, this shouldn't affect performance too much.
    // Latency is bound by both the size of the buffer and the latency of the device (whichever is higher)

    // the size of the rtaudio buffer should be relative to the period of the renderer
    // if the renderer's period is 5ms, then there will be audio available in the buffer
    // every 5 ms +/- 1 ms.

    RtAudio::StreamOptions streamOpts;
    streamOpts.numberOfBuffers = 2;
    streamOpts.streamName = "Trackerboy";

    RtAudio::StreamParameters streamParameters;
    streamParameters.deviceId = rtaudioDevice;
    streamParameters.nChannels = 2;
    unsigned bufferFrames = config.period * samplerate / 1000;


    try {
        auto handle = api->access();
        handle->openStream(
            &streamParameters,
            nullptr,
            RTAUDIO_SINT16,
            samplerate,
            &bufferFrames,
            audioCallback,
            this,
            &streamOpts,
            errorCallback
        );
    } catch (RtAudioError const& err) {
        // the device does not support our settings or some other error occured
        handleError("setConfig: could not open stream:", err);
        return;
    }

    qDebug() << LOG_PREFIX << "rtaudio buffer (in samples):" << bufferFrames;

    if (running) {
        try {
            QMutexLocker locker(&mMutex);
            _start(api);
        } catch (RtAudioError const& err) {
            handleError("setConfig: could not restart stream:", err);
            return;
        }
    }

}

bool AudioStream::start() {
    QMutexLocker locker(&mMutex);
    if (_isEnabled() && !_isRunning()) {
        try {
            _start(mApi);
        } catch (RtAudioError const& err) {
            locker.unlock();
            handleError("error occured when starting stream:", err);
            return false;
        }
    }

    return true;
}

void AudioStream::_start(Guarded<RtAudio> *api) {
    mBuffer.reset();
    mPlaybackDelay = mBuffer.size();

    auto handle = api->access();
    handle->setStreamTime(0.0);
    handle->startStream();
}

bool AudioStream::stop() {
    QMutexLocker locker(&mMutex);
    if (_isRunning()) {
        try {
            auto handle = mApi->access();
            handle->stopStream();
        } catch (RtAudioError const& err) {
            locker.unlock();
            handleError("error occured when stopping stream:", err);
            return false;
        }
    }

    return true;
}


void AudioStream::disable() {
    QMutexLocker locker(&mMutex);
    _disable();
}

void AudioStream::_close() {
    
    if (_isEnabled()) {
        auto handle = mApi->access();
        if (handle->isStreamOpen()) {
            handle->closeStream();
        }
    }
}

void AudioStream::_disable() {
    _close();
    mApi = nullptr;
}


int AudioStream::audioCallback(
    void *output,
    void *input,
    unsigned bufferFrames,
    double streamTime,
    RtAudioStreamStatus status,
    void *userData
) {
    Q_UNUSED(input) // only outputing sound
    return static_cast<AudioStream*>(userData)->handleAudio(
        static_cast<int16_t*>(output), 
        bufferFrames, 
        streamTime, 
        status
    );
}

int AudioStream::handleAudio(int16_t *output, size_t bufferFrames, double streamTime, RtAudioStreamStatus status) {
    Q_UNUSED(streamTime) // not used for timing or anything, we're just copying out from a buffer

    if (status == RTAUDIO_OUTPUT_UNDERFLOW) {
        ++mUnderruns;
    }

    if (mPlaybackDelay) {
        auto samples = std::min(mPlaybackDelay, bufferFrames);
        bufferFrames -= samples;
        std::fill_n(output, samples * 2, (int16_t)0);
        output += samples * 2;
        mPlaybackDelay -= samples;
    }

    auto reader = mBuffer.reader();

    auto nread = reader.fullRead(output, bufferFrames);
    if (nread != bufferFrames) {
        std::fill_n(output + (nread * 2), bufferFrames - nread, (int16_t)0);
    }


    return 0;
}


void AudioStream::handleError(const char *msg, RtAudioError const& err) {
    qCritical().noquote()
        << LOG_PREFIX
        << msg
        << QString::fromStdString(err.getMessage());

    QMutexLocker locker(&mMutex);
    
    _disable();
    mLastError = err.getType();
}
