
#include "core/audio/AudioStream.hpp"
#include "core/audio/AudioProber.hpp"
#include "core/config/SoundConfig.hpp"

#include <QtDebug>

#include <algorithm>


#define TU AudioStreamTU
namespace TU {

static const char* LOG_PREFIX = "[AudioStream]";

}


AudioStream::AudioStream() :
    QObject(),
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
    disable();
}


bool AudioStream::isEnabled() {
    return mEnabled;
}

bool AudioStream::isRunning() {
    return isEnabled() && mRunning.load();
}

int AudioStream::underruns() const {
    return mUnderruns.load();
}

void AudioStream::resetUnderruns() {
    mUnderruns = 0;
}

size_t AudioStream::bufferSize() {
    return mBuffer.size();
}

double AudioStream::elapsed() {
    return 0.0;
}

AudioRingbuffer::Writer AudioStream::writer() {
    return mBuffer.writer();
}

void AudioStream::setConfig(SoundConfig const& config) {

    auto const samplerate = config.samplerate();
    
    auto &prober = AudioProber::instance();
    
    auto deviceConfig = ma_device_config_init(ma_device_type_playback);
    // always 32-bit float stereo format
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = 2;
    deviceConfig.dataCallback = deviceDataCallback;
    deviceConfig.stopCallback = deviceStopCallback;
    deviceConfig.pUserData = this;
    deviceConfig.sampleRate = samplerate;
    deviceConfig.playback.pDeviceID = prober.deviceId(config.backendIndex(), config.deviceIndex());

    // get the current running state
    // if we are running then we will have to start the newly opened stream
    bool running = isRunning();

    // must be disabled when applying config
    disable();

    // update buffer size
    mBuffer.init((size_t)(config.latency() * samplerate / 1000));

    // attempt to initialize device
    auto result = ma_device_init(prober.context(config.backendIndex()), &deviceConfig, mDevice.get());
    if (result != MA_SUCCESS) {
        handleError("could not initialize device:", result);
        return;
    }

    mEnabled = true;

    if (running) {
        start();
    }

}

bool AudioStream::start() {
    if (isEnabled() && !isRunning()) {
        mBuffer.reset();
        mPlaybackDelay = mBuffer.size();
        
        auto result = ma_device_start(mDevice.get());
        if (result != MA_SUCCESS) {
            handleError("failed to start device:", result);
            return false;
        }
        mRunning = true;
    }

    return true;
}

bool AudioStream::stop() {
    if (isRunning()) {
        mRunning = false;
        auto result = ma_device_stop(mDevice.get());
        if (result != MA_SUCCESS) {
            handleError("failed to stop device:", result);
            return false;
        }
    }

    return true;
}


void AudioStream::disable() {
    mRunning = false;

    if (mEnabled) {
        mEnabled = false;
        ma_device_uninit(mDevice.get());
    }

}

void AudioStream::deviceDataCallback(ma_device *device, void *out, const void *in, ma_uint32 frames) {
    Q_UNUSED(in)

    static_cast<AudioStream*>(device->pUserData)->handleData(
        static_cast<float*>(out),
        (size_t)frames
    );
}

void AudioStream::handleData(float *out, size_t frames) {
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
    // this handler is called:
    //  * implicitly: due to device error or disconnect (mRunning = true)
    //  * explicitly: when the stream is stopped via stop() (mRunning = false)
    
    if (mRunning) {
        qCritical() << TU::LOG_PREFIX << "stream aborted";
        mRunning = false;
        // we didn't trigger this stop via stop(), an error must've occurred
        emit aborted();
    }
}


void AudioStream::handleError(const char *msg, ma_result err) {
    qCritical().noquote()
        << TU::LOG_PREFIX
        << msg
        << ma_result_description(err);
    disable();
}

#undef TU
