
#include "audio/AudioStream.hpp"
#include "verdigris/wobjectimpl.h"

#include <QtDebug>

#include <algorithm>


#define TU AudioStreamTU
namespace TU {

static const char* LOG_PREFIX = "[AudioStream]";

}

W_OBJECT_IMPL(AudioStream)


AudioStream::MaDeviceWrapper::MaDeviceWrapper() :
    mInitialized(false),
    mDevice()
{
}

AudioStream::MaDeviceWrapper::~MaDeviceWrapper() {
    if (mInitialized) {
        uninit();
    }
}

ma_result AudioStream::MaDeviceWrapper::init(ma_context *ctx, ma_device_config const* config) {
    auto result = ma_device_init(ctx, config, &mDevice);
    mInitialized = result == MA_SUCCESS;
    return result;
}

void AudioStream::MaDeviceWrapper::uninit() {
    ma_device_uninit(&mDevice);
}

ma_device* AudioStream::MaDeviceWrapper::get() {
    return &mDevice;
}


AudioStream::AudioStream(QObject *parent) :
    QObject(parent),
    mEnabled(false),
    mRunning(false),
    mBuffer(),
    mContext(),
    mDevice(),
    mPlaybackDelay(0),
    mUnderruns(0),
    mDraining(false)
{

}

bool AudioStream::isEnabled() const {
    return mEnabled;
}

bool AudioStream::isRunning() const {
    return isEnabled() && mRunning.load();
}

unsigned AudioStream::underruns() const {
    return mUnderruns.load();
}

void AudioStream::resetUnderruns() {
    mUnderruns = 0;
}

size_t AudioStream::bufferSize() const {
    return mBuffer.size();
}

void AudioStream::setDraining(bool draining) {
    mDraining = draining;
}

AudioRingbuffer::Writer AudioStream::writer() {
    return mBuffer.writer();
}

void AudioStream::open(AudioEnumerator::Device const& device, int samplerate, int latency) {

    // get the current running state
    // if we are running then we will have to start the newly opened stream
    bool running = isRunning();

    // must be disabled when changing settings
    disable();

    // update buffer size
    mBuffer.init((size_t)(latency * samplerate / 1000));

    auto deviceConfig = ma_device_config_init(ma_device_type_playback);
    // always 32-bit float stereo format
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = 2;
    deviceConfig.dataCallback = deviceDataCallback;
    deviceConfig.stopCallback = deviceStopCallback;
    deviceConfig.pUserData = this;
    deviceConfig.sampleRate = samplerate;
    deviceConfig.playback.pDeviceID = device.id;

    mContext = device.context;
    auto result = mDevice.init(mContext.get(), &deviceConfig);
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
        mDraining = false;
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
        mDevice.uninit();
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

    // an entire buffer's worth of silence is played when the stream is started
    // this gives the us ample time to fill the buffer before playing from it.
    // Without this the output might be choppy at the start.

    if (mPlaybackDelay) {
        auto samples = std::min(mPlaybackDelay, frames);
        frames -= samples;
        // miniaudio clears the output buffer before calling the callback
        // so just seek the output pointer
        out += samples * 2;
        mPlaybackDelay -= samples;
    }

    auto nread = mBuffer.reader().fullRead(out, frames);
    if (nread < frames && !mDraining) {
        ++mUnderruns;
    }
}

void AudioStream::deviceStopCallback(ma_device *device) {
    // on explicit stops, abort does nothing, since isRunning() = false
    static_cast<AudioStream*>(device->pUserData)->handleStop();
}

void AudioStream::handleStop() {
    // this handler is called:
    //  * implicitly: due to device error or disconnect (mRunning = true)
    //  * explicitly: when the stream is stopped via stop() (mRunning = false)

    if (mRunning) {
        qCritical() << TU::LOG_PREFIX << "stream aborted";
        mRunning = false;
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
