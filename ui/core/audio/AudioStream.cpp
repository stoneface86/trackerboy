
#include "core/audio/AudioStream.hpp"
#include "core/audio/AudioProber.hpp"
#include "core/samplerates.hpp"

#include <QMutexLocker>
#include <QtDebug>

#include <algorithm>



static const char* LOG_PREFIX = "[AudioStream]";


AudioStream::AudioStream() :
    mMutex(),
    mEnabled(false),
    mDevice(new ma_device),
    mBuffer(),
    mSamplerate(0),
    mRunning(false),
    mPlaybackDelay(0),
    mUnderruns(0)
{
}

AudioStream::~AudioStream() {
    QMutexLocker locker(&mMutex);
    _disable();
}


bool AudioStream::isEnabled() {
    QMutexLocker locker(&mMutex);
    return _isEnabled();
}

bool AudioStream::_isEnabled() {
    return mEnabled;
}

bool AudioStream::isRunning() {
    QMutexLocker locker(&mMutex);
    return _isRunning();
}

bool AudioStream::_isRunning() {
    if (_isEnabled()) {
        return mRunning;
    } else {
        return false;
    }
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
    return 0.0;
}

AudioRingbuffer::Writer AudioStream::writer() {
    QMutexLocker locker(&mMutex);
    return mBuffer.writer();
}

void AudioStream::setConfig(Config::Sound const& config) {

    auto const samplerate = SAMPLERATE_TABLE[config.samplerateIndex];
    
    auto &prober = AudioProber::instance();
    
    auto deviceConfig = ma_device_config_init(ma_device_type_playback);
    // always 16-bit stereo format
    deviceConfig.playback.format = ma_format_s16;
    deviceConfig.playback.channels = 2;
    deviceConfig.dataCallback = deviceDataCallback;
    deviceConfig.stopCallback = deviceStopCallback;
    deviceConfig.pUserData = this;
    deviceConfig.sampleRate = samplerate;
    deviceConfig.playback.pDeviceID = prober.deviceId(config.backendIndex, config.deviceIndex);
    
    mMutex.lock();

    // get the current running state
    // if we are running then we will have to start the newly opened stream
    bool running = _isRunning();

    // must be disabled when applying config
    _disable();

    // update buffer size
    mBuffer.init((size_t)(config.latency * samplerate / 1000));

    // attempt to initialize device
    auto result = ma_device_init(prober.context(config.backendIndex), &deviceConfig, mDevice.get());
    if (result != MA_SUCCESS) {
        _handleError("could not initialize device:", result);
        return;
    }

    mEnabled = true;
    mMutex.unlock();

    if (running) {
        start();
    }

    
    #if 0

    
    // get the configured device
    auto dev = config.deviceConfig.device();
    if (dev == nullptr) {
        // device is not available
        qCritical() << LOG_PREFIX << "setConfig: the configured device is not available";
        disable();
        return;
    }

    mMutex.lock();

    // get the current running state
    // if we are running then we will have to start the newly opened stream
    bool running = _isRunning();

    _disable();

    // apply the configuration

    // initialize the internal buffer

    mBuffer.init((size_t)(config.latency * samplerate / 1000));
    mMutex.unlock();

    // open a stream
    auto stream = soundio_outstream_create(dev);
    stream->sample_rate = samplerate;
    stream->name = "trackerboy";
    stream->format = SoundIoFormatS16NE;
    stream->write_callback = streamWriteCallback;
    stream->userdata = this;

    auto err = soundio_outstream_open(stream);
    if (err != SoundIoErrorNone) {
        qCritical().noquote()
            << LOG_PREFIX 
            << "setConfig: failed to open stream for device: " 
            << soundio_strerror(err);

        soundio_device_unref(dev);
        // still disabled, just leave
        return;
    }

    qDebug() << LOG_PREFIX << "software latency:" << stream->software_latency;

    mMutex.lock();
    mDevice = dev;
    mOutStream = stream;
    mMutex.unlock();

    if (running) {
        start();
    }


    #endif

}

bool AudioStream::start() {
    QMutexLocker locker(&mMutex);
    if (_isEnabled() && !_isRunning()) {
        mBuffer.reset();
        mPlaybackDelay = mBuffer.size();
        
        auto result = ma_device_start(mDevice.get());
        if (result != MA_SUCCESS) {
            _handleError("failed to start device:", result);
            return false;
        }
        mRunning = true;
    }

    return true;
}

bool AudioStream::stop() {
    QMutexLocker locker(&mMutex);
    if (_isRunning()) {
        mRunning = false;
        auto result = ma_device_stop(mDevice.get());
        if (result != MA_SUCCESS) {
            _handleError("failed to stop device:", result);
            return false;
        }
    }

    return true;
}


void AudioStream::disable() {
    QMutexLocker locker(&mMutex);
    _disable();
}

void AudioStream::_disable() {
    mRunning = false;

    if (mEnabled) {
        mEnabled = false;
        ma_device_uninit(mDevice.get());
    }

}

void AudioStream::deviceDataCallback(ma_device *device, void *out, const void *in, ma_uint32 frames) {
    Q_UNUSED(in)

    static_cast<AudioStream*>(device->pUserData)->handleData(
        static_cast<int16_t*>(out),
        (size_t)frames
    );
}

void AudioStream::handleData(int16_t *out, size_t frames) {
    if (mPlaybackDelay) {
        auto samples = std::min(mPlaybackDelay, frames);
        frames -= samples;
        std::fill_n(out, samples * 2, (int16_t)0);
        out += samples * 2;
        mPlaybackDelay -= samples;
    }

    auto reader = mBuffer.reader();
    reader.fullRead(out, frames);
    
}

void AudioStream::deviceStopCallback(ma_device *device) {
    static_cast<AudioStream*>(device->pUserData)->handleStop();
}

void AudioStream::handleStop() {

}


void AudioStream::_handleError(const char *msg, ma_result err) {
    qCritical().noquote()
        << LOG_PREFIX
        << msg
        << ma_result_description(err);
    _disable();
}
