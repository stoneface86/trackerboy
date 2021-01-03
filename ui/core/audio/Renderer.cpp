
#include "core/audio/Renderer.hpp"
#include "core/samplerates.hpp"

#include <QMutexLocker>
#include <QtDebug>

#include <cstddef>
#include <algorithm>


Renderer::Renderer(
    Miniaudio &miniaudio,
    Spinlock &spinlock,
    trackerboy::InstrumentTable &itable,
    trackerboy::WaveTable &wtable,
    InstrumentListModel &instrumentModel,
    SongListModel &songModel,
    WaveListModel &waveModel
) :
    mMiniaudio(miniaudio),
    mSpinlock(spinlock),
    mAudioStopCondition(),
    mMutex(),
    mBackgroundThread(nullptr),
    mRunning(false),
    mStopBackground(false),
    mDevice(),
    mSynth(44100),
    mRc(mSynth.apu(), itable, wtable),
    mEngine(mRc),
    mIr(mRc),
    mInstrumentModel(instrumentModel),
    mSongModel(songModel),
    mWaveModel(waveModel),
    mBuffer(),
    mPreviewState(PreviewState::none),
    mPreviewChannel(trackerboy::ChType::ch1),
    mStopCounter(0),
    mStopped(false),
    mLockFails(0),
    mUnderruns(0)
{
    mBackgroundThread = QThread::create(&Renderer::backgroundThreadRun, this);
    mBackgroundThread->start();
}

Renderer::~Renderer() {
    // stop the background thread
    mMutex.lock();
    mStopBackground = true;
    mMutex.unlock();

    // wake and wait for the background thread to finish
    mAudioStopCondition.wakeAll();
    mBackgroundThread->wait();
    delete mBackgroundThread;

    closeDevice();
}

unsigned Renderer::lockFails() const {
    return mLockFails.load();
}

unsigned Renderer::underruns() const {
    return mUnderruns.load();
}

unsigned Renderer::elapsed() const {
    return mSamplesElapsed.load() * 1000 / mDevice.value().sampleRate;
}

unsigned Renderer::bufferUsage() const {
    return mBufferUsage.load();
}

unsigned Renderer::bufferSize() const {
    return mBuffer.size();
}

ma_device const& Renderer::device() const{
    return mDevice.value();
}

AudioRingbuffer& Renderer::returnBuffer() {
    return mReturnBuffer;
}

bool Renderer::isRunning() {
    QMutexLocker locker(&mMutex);
    return mRunning;
}

void Renderer::setConfig(Config::Sound const &soundConfig) {
    closeDevice();

    QMutexLocker locker(&mMutex);

    auto const SAMPLERATE = SAMPLERATE_TABLE[soundConfig.samplerateIndex];

    auto config = ma_device_config_init(ma_device_type_playback);
    config.playback.pDeviceID = mMiniaudio.deviceId(soundConfig.deviceIndex);
    // always 16-bit stereo format
    config.playback.format = ma_format_s16;
    config.playback.channels = 2;
    config.performanceProfile = soundConfig.lowLatency ? ma_performance_profile_low_latency : ma_performance_profile_conservative;
    config.sampleRate = SAMPLERATE;
    config.dataCallback = audioCallbackRun;
    config.pUserData = this;

    // initialize device with settings
    mDevice.emplace();
    auto err = ma_device_init(mMiniaudio.context(), &config, &mDevice.value());
    assert(err == MA_SUCCESS);

    auto &device = mDevice.value();
    mReturnBuffer.init(device.playback.internalPeriodSizeInFrames * device.playback.internalPeriods);

    mSynth.setSamplingRate(SAMPLERATE);
    mSynth.setVolume(soundConfig.volume);
    mSynth.setupBuffers();

    mBuffer.setSize(soundConfig.buffersize, mSynth.framesize());
}

void Renderer::closeDevice() {
    QMutexLocker locker(&mMutex);

    if (mDevice) {
        ma_device_uninit(&mDevice.value());
        mDevice.reset();
    }
}

void Renderer::startDevice() {
    QMutexLocker locker(&mMutex);

    if (!mRunning) {
        mStopCounter = 0;
        mStopped = false;
        mBuffer.reset();
        mSamplesElapsed = 0;

        ma_device_start(&mDevice.value());
        mRunning = true;
    }
}

void Renderer::stopMusic() {

}

void Renderer::previewInstrument(trackerboy::Note note) {
    mSpinlock.lock();
    switch (mPreviewState) {
        case PreviewState::waveform:
            resetPreview();
            [[fallthrough]];
        case PreviewState::none:
            {
                auto inst = mInstrumentModel.instrument(mInstrumentModel.currentIndex());
                mIr.setInstrument(*inst);
                mPreviewState = PreviewState::instrument;
                mPreviewChannel = static_cast<trackerboy::ChType>(inst->data().channel);
            }
            mEngine.unlock(mPreviewChannel);
            [[fallthrough]];
        case PreviewState::instrument:
            mIr.playNote(note);
            break;
    }
    mSpinlock.unlock();

    startDevice();
}

void Renderer::previewWaveform(trackerboy::Note note) {
    mSpinlock.lock();

    switch (mPreviewState) {
        case PreviewState::instrument:
            resetPreview();
            [[fallthrough]];
        case PreviewState::none:
            mPreviewState = PreviewState::waveform;
            mPreviewChannel = trackerboy::ChType::ch3;
            mEngine.unlock(trackerboy::ChType::ch3);
            trackerboy::ChannelControl::writePanning(trackerboy::ChType::ch3, mRc, 0x11);
            trackerboy::ChannelControl::writeWaveram(mRc, *(mWaveModel.currentWaveform()));
            // volume = 100%
            mRc.apu.writeRegister(gbapu::Apu::REG_NR32, 0x20);
            // retrigger
            mRc.apu.writeRegister(gbapu::Apu::REG_NR34, 0x80);

            [[fallthrough]];
        case PreviewState::waveform:
            if (note > trackerboy::NOTE_LAST) {
                // should never happen, but clamp just in case
                note = trackerboy::NOTE_LAST;
            }
            trackerboy::ChannelControl::writeFrequency(
                trackerboy::ChType::ch3,
                mRc,
                trackerboy::NOTE_FREQ_TABLE[note]
            );
            break;
    }

    mSpinlock.unlock();

    startDevice();
}

void Renderer::stopPreview() {
    mSpinlock.lock();
    if (mPreviewState != PreviewState::none) {
        resetPreview();
    }
    mSpinlock.unlock();
}

void Renderer::resetPreview() {
    // lock the channel so it can be used for music
    trackerboy::ChannelControl::writePanning(mPreviewChannel, mRc, 0);
    mEngine.lock(mPreviewChannel);
    mPreviewState = PreviewState::none;
}

// ~~~~~~ BACKGROUND THREAD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void Renderer::backgroundThreadRun(Renderer *renderer) {
    qDebug() << "[Audio background] thread started";
    renderer->handleBackground();
    qDebug() << "[Audio background] thread stopped";
}

void Renderer::handleBackground() {
    // the audio callback will signal the condition variable when there's
    // nothing to play.

    mMutex.lock();
    for (;;) {
        
        if (mStopBackground) {
            mMutex.unlock();
            break;
        }

        qDebug() << "[Audio background] waiting...";

        // wait here, we stop waiting if
        // 1. the callback thread has finished and is signalling us to kill it
        // 2. the Renderer is being destroyed and we must exit the loop
        mAudioStopCondition.wait(&mMutex);

        // kill the callback thread
        if (mRunning) {
            qDebug() << "[Audio background] Stopping device";
            ma_device_stop(&mDevice.value());
            mRunning = false;
        }

    }
}

// ~~~~~~ CALLBACK THREAD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void Renderer::audioCallbackRun(
    ma_device *device,
    void *out,
    const void *in,
    ma_uint32 frames
) {
    (void)in;
    static_cast<Renderer*>(device->pUserData)->handleCallback(
        reinterpret_cast<int16_t*>(out),
        frames
    );
}

constexpr int STOP_FRAMES = 5;

void Renderer::handleCallback(int16_t *out, size_t frames) {

    mBufferUsage = mBuffer.framesInQueue();

    while (frames) {

        if (mStopped) {
            if (mBuffer.isEmpty()) {
                mAudioStopCondition.wakeOne();
                return;
            }

        } else if (mBuffer.framesToQueue()) {
            // attempt to generate a frame and queue it for playback

            if (mSpinlock.tryLock()) {

                if (mStopCounter) {
                    if (--mStopCounter == 0) {
                        mStopped = true;
                    }
                } else {

                    trackerboy::Frame frameInfo;
                    mEngine.step(frameInfo);

                    if (mPreviewState == PreviewState::instrument) {
                        mIr.step();
                    }

                    if (frameInfo.halted && mPreviewState == PreviewState::none) {
                        mStopCounter = STOP_FRAMES;
                    }

                }
                auto framesize = mSynth.run();
                mBuffer.queueFrame(mSynth.buffer(), framesize);

                mSpinlock.unlock();
            } else {
                // failed to lock spinlock, do nothing and increment counter for diagnostics
                ++mLockFails;
                if (mBuffer.isEmpty()) {
                    // give up, underrun will occur
                    ++mUnderruns;
                    break;
                }
            }
        }


        size_t framesRead = mBuffer.read(out, frames);
        frames -= framesRead;

        // write what we just read to the return buffer for visualizers
        auto samplesRead = framesRead * 2;
        mReturnBuffer.fullWrite(out, framesRead);


        out += samplesRead;

        mSamplesElapsed += framesRead;
    }

}
