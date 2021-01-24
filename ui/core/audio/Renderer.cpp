
#include "core/audio/Renderer.hpp"
#include "core/samplerates.hpp"

#include <QMutexLocker>
#include <QtDebug>

#include <cstddef>
#include <algorithm>

// TODO:
// 1. optimize atomic variables if possible (all currently use the default memory order seq_cst)
// 2. the return buffer must be filled with silence when an underrun occurs
// 3. the return buffer should be cleared on device stop
// 4. implement stopDevice()


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
    mInstrumentModel(instrumentModel),
    mSongModel(songModel),
    mWaveModel(waveModel),
    mIdleCondition(),
    mMutex(),
    mBackgroundThread(nullptr),
    mRunning(false),
    mStopBackground(false),
    mStopDevice(false),
    mSync(false),
    mDevice(),
    mLockFails(0),
    mUnderruns(0),
    mSamplesElapsed(0),
    mBufferUsage(0),
    mLatency(0),
    mSynth(44100),
    mRc(mSynth.apu(), itable, wtable),
    mEngine(mRc),
    mIr(mRc),
    mBuffer(),
    mPreviewState(PreviewState::none),
    mPreviewChannel(trackerboy::ChType::ch1),
    mStopCounter(0),
    mShouldStop(false),
    mStopped(false),
    mSyncCounter(0),
    mSyncPeriod(1),
    mReturnBuffer(),
    mLastFrameOut(),
    mFrameLock()
{
    mBackgroundThread = QThread::create(&Renderer::backgroundThreadRun, this);
    mBackgroundThread->start(QThread::TimeCriticalPriority);
}

Renderer::~Renderer() {
    // stop the background thread
    mMutex.lock();
    mStopBackground = true;
    mStopDevice = true;
    mMutex.unlock();

    // wake and wait for the background thread to finish
    mIdleCondition.wakeAll();
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

long long Renderer::lastSyncTime() {
    QMutexLocker locker(&mMutex);
    return mLatency.count();
}

ma_device const& Renderer::device() const{
    return mDevice.value();
}

AudioRingbuffer& Renderer::returnBuffer() {
    return mReturnBuffer;
}

RenderFrame const& Renderer::acquireCurrentFrame() {
    mFrameLock.lock();
    return mLastFrameOut;
}

void Renderer::releaseFrame() {
    mFrameLock.unlock();
}

bool Renderer::isRunning() {
    QMutexLocker locker(&mMutex);
    return mRunning;
}

void Renderer::setConfig(Config::Sound const &soundConfig) {
    closeDevice();

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

    mSyncPeriod = mDevice.value().playback.internalPeriodSizeInFrames;

    mReturnBuffer.init(SAMPLERATE);
    mSynth.setSamplingRate(SAMPLERATE);
    mSynth.setVolume(soundConfig.volume);
    mSynth.setupBuffers();

    mBuffer.setSize(soundConfig.buffersize, mSynth.framesize());

    if (mRunning) {
        // the callback was running before we applied the config, restart it

        ma_device_start(&mDevice.value());
    }

}

void Renderer::closeDevice() {

    if (mDevice) {
        stopDevice();
        ma_device_uninit(&mDevice.value());
        mDevice.reset();
    }
}

void Renderer::startDevice() {
    QMutexLocker locker(&mMutex);

    // the callback thread will output STOP_FRAMES before shutting down, this
    // this flag will cancel the countdown if it is active
    mCancelStop = true;
    if (!mRunning) {

        // callback initial state
        mStopped = false;
        mShouldStop = false;
        mStopCounter = 0;
        mSyncCounter = 0;
        mBuffer.reset();
        mSamplesElapsed = 0;

        ma_device_start(&mDevice.value());
        mRunning = true;
        locker.unlock();
        emit audioStarted();

        mIdleCondition.wakeAll();
    }
}

void Renderer::stopDevice() {
    // TODO
}

void Renderer::playMusic(uint8_t orderNo, uint8_t rowNo) {
    mSpinlock.lock();
    mEngine.play(*mSongModel.currentSong(), orderNo, rowNo);
    mSpinlock.unlock();

    startDevice();
}

// SLOTS

void Renderer::clearDiagnostics() {
    mLockFails = 0;
    mUnderruns = 0;
    mSamplesElapsed = 0;
}

void Renderer::play() {
    playMusic(mSongModel.orderModel().currentPattern(), 0);
}

void Renderer::playPattern() {
    // TODO: Engine needs functionality for looping a single pattern
}

void Renderer::playFromCursor() {
    // TODO: we need a way to get the cursor row from the PatternEditor
}

void Renderer::playFromStart() {
    playMusic(0, 0);
}


void Renderer::previewInstrument(trackerboy::Note note) {
    mSpinlock.lock();
    switch (mPreviewState) {
        case PreviewState::waveform:
            resetPreview();
            [[fallthrough]];
        case PreviewState::none:
            {
                // set instrument runtime's instrument to the current one
                auto inst = mInstrumentModel.instrument(mInstrumentModel.currentIndex());
                mIr.setInstrument(*inst);
                mPreviewState = PreviewState::instrument;
                mPreviewChannel = static_cast<trackerboy::ChType>(inst->data().channel);
            }
            // unlock the channel for preview
            mEngine.unlock(mPreviewChannel);
            [[fallthrough]];
        case PreviewState::instrument:
            // update the current note
            mIr.playNote(note);
            break;
    }
    mSpinlock.unlock();

    startDevice();
}

void Renderer::previewWaveform(trackerboy::Note note) {
    mSpinlock.lock();

    // state changes
    // instrument -> none -> waveform

    switch (mPreviewState) {
        case PreviewState::instrument:
            resetPreview();
            [[fallthrough]];
        case PreviewState::none:
            mPreviewState = PreviewState::waveform;
            mPreviewChannel = trackerboy::ChType::ch3;
            // unlock the channel, no longer effected by music
            mEngine.unlock(trackerboy::ChType::ch3);
            // middle panning for CH3
            trackerboy::ChannelControl::writePanning(trackerboy::ChType::ch3, mRc, 0x11);
            // set the waveram with the waveform we are previewing
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

void Renderer::stopMusic() {
    mSpinlock.lock();
    mEngine.halt();
    mSpinlock.unlock();
}



void Renderer::resetPreview() {
    // lock the channel so it can be used for music
    trackerboy::ChannelControl::writePanning(mPreviewChannel, mRc, 0);
    mEngine.lock(mPreviewChannel);
    mPreviewState = PreviewState::none;
}

// ~~~~~~ BACKGROUND THREAD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// This thread runs alongside the callback thread, for synchronizing audio with
// the GUI and stopping the callback thread when done rendering (the callback thread
// cannot stop itself).

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
        
        // wait here, we stop waiting if
        // 1. the callback thread was started and we need to keep the GUI in sync
        // 2. the Renderer is being destroyed and we must exit the loop
        mIdleCondition.wait(&mMutex);
        
        // [2]
        if (mStopBackground) {
            break;
        }
        
        // [1]
        if (mRunning) {

            mMutex.unlock();

            // NOTE: audio sync is temporary, eventually the GUI will update via timer
            // according to the module's framerate setting.

            // synchronization
            auto lastSyncTime = Clock::now();
            mCallbackMutex.lock();
            
            do {

                // we have to poll for a sync event because waking this thread from
                // the callback thread may cause glitches (we don't know how long QWaitCondition::wakeOne takes to execute)
                if (mSync) {
                    emit audioSync();
                    mSync = false;
                    auto currentSyncTime = Clock::now();
                    mMutex.lock();
                    mLatency = currentSyncTime - lastSyncTime;
                    mMutex.unlock();
                    lastSyncTime = currentSyncTime;
                }

                // wait at least 1 millisecond
                // timing accuracy of the sync event depends on the OS scheduler
                // should only be inaccurate for latency < 15ms (cough windows cough)
                mCallbackCondition.wait(&mCallbackMutex, QDeadlineTimer(std::chrono::milliseconds(1), Qt::PreciseTimer));
                
            } while (!mStopDevice);
            
            mCallbackMutex.unlock();

            mMutex.lock();

            // stopping, kill the callback thread
            mStopDevice = false;
            ma_device_stop(&mDevice.value());
            mRunning = false;
            mEngine.reset();
            emit audioStopped();

        }

        

    }

    mMutex.unlock();
}

// ~~~~~~ CALLBACK THREAD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// called when audio data is needed to be played out to speakers. All rendering
// is done here.

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

    if (mStopped) {
        // keep waking the background thread in case of lost wakeup
        mCallbackCondition.wakeOne();
        return;
    }

    mBufferUsage = mBuffer.framesInQueue();

    while (frames) {

        // check if the user immediately previewed/played music after stopping
        if (mCancelStop) {
            mStopCounter = 0;
            mShouldStop = false;
            mCancelStop = false;
        }

        if (mShouldStop) {
            // do not render anything if we should stop
            if (mBuffer.isEmpty()) {
                // buffer is now empty, signal to the background thread that we are done

                // we can use a mutex here, as we no longer care about audio glitches
                // at this point
                mCallbackMutex.lock();
                mStopDevice = true;
                mCallbackMutex.unlock();

                mStopped = true;
                mCallbackCondition.wakeOne();
                return;
            }
        } else if (mBuffer.framesToQueue()) {
            // attempt to generate a frame and queue it for playback

            if (mSpinlock.tryLock()) {

                RenderFrame rFrame;

                if (mStopCounter) {
                    if (--mStopCounter == 0) {
                        mShouldStop = true;
                    }
                    // the GUI should ignore this frame on audio sync event
                    rFrame.ignore = true;
                } else {

                    // step the engine
                    mEngine.step(rFrame.engineFrame);

                    // step the instrument runtime if we are previewing an instrument
                    if (mPreviewState == PreviewState::instrument) {
                        mIr.step();
                    }

                    // begin the stop countdown if the engine halted and we are not previewing anything
                    if (rFrame.engineFrame.halted && mPreviewState == PreviewState::none) {
                        mStopCounter = STOP_FRAMES;
                    }

                    // do not ignore this frame
                    rFrame.ignore = false;
                }

                // get the current register dump
                rFrame.registers = mSynth.apu().registers();

                // synthesize the frame
                rFrame.nsamples = mSynth.run();

                // queue it for future playback
                mBuffer.queueFrame(mSynth.buffer(), rFrame);

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

        // write buffered data to the speakers
        size_t framesRead = mBuffer.read(out, frames);
        frames -= framesRead;

        if (mBuffer.hasNewFrame()) {
            if (mFrameLock.tryLock()) {
                mLastFrameOut = mBuffer.popFrame();
                mFrameLock.unlock();
                
            }
        }

        // write what we sent to the speakers for visualizers
        mReturnBuffer.fullWrite(out, framesRead);

        auto samplesRead = framesRead * 2;
        out += samplesRead;
        mSyncCounter += framesRead;

        mSamplesElapsed += (unsigned)framesRead;
    }

    if (mSyncCounter >= mSyncPeriod) {
        // sync audio when we have written at least mSyncPeriod samples
        mSync = true;
        mSyncCounter %= mSyncPeriod;
    }
}
