
#include "core/audio/Renderer.hpp"
#include "core/samplerates.hpp"

#include <QDeadlineTimer>
#include <QMutexLocker>
#include <QtDebug>

#include <algorithm>
#include <chrono>
#include <type_traits>

Renderer::Renderer(
    Miniaudio &miniaudio,
    Spinlock &spinlock,
    ModuleDocument &document,
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
    mCallbackCondition(),
    mMutex(),
    mCallbackMutex(),
    mBackgroundThread(nullptr),
    mRunning(false),
    mStopBackground(false),
    mStopDevice(false),
    mSync(false),
    mCancelStop(false),
    mDevice(),
    mSynth(44100),
    mRc(mSynth.apu(), document.instrumentTable(), document.waveTable()),
    mEngine(mRc),
    mIr(mRc),
    mPreviewState(PreviewState::none),
    mPreviewChannel(trackerboy::ChType::ch1),
    mCallbackState(CallbackState::stopped),
    mStopCounter(0),
    mBuffer(),
    mReturnBuffer(),
    mLockFails(0),
    mUnderruns(0),
    mSamplesElapsed(0)
{
    mBackgroundThread.reset(QThread::create(&Renderer::backgroundThreadRun, this));
    mBackgroundThread->start();
}

Renderer::~Renderer() {
    // stop the background thread
    mMutex.lock();
    mCallbackMutex.lock();

    mStopBackground = true;
    mStopDevice = true;
    mIdleCondition.wakeOne();
    mCallbackCondition.wakeOne();

    mCallbackMutex.unlock();
    mMutex.unlock();


    // wait for the background thread to finish
    mBackgroundThread->wait();

    closeDevice();
}

Renderer::Diagnostics Renderer::diagnostics() {
    QMutexLocker locker(&mMutex);
    return {
        mLockFails.load(),
        mUnderruns.load(),
        mSamplesElapsed.load()
    };
}

ma_device const& Renderer::device() const{
    return mDevice.value();
}

AudioRingbuffer::Reader Renderer::returnBuffer() {
    return mReturnBuffer.reader();
}

bool Renderer::isRunning() {
    QMutexLocker locker(&mMutex);
    return mRunning;
}

void Renderer::setConfig(Config::Sound const &soundConfig) {
    QMutexLocker locker(&mMutex);
    closeDevice();

    auto const SAMPLERATE = SAMPLERATE_TABLE[soundConfig.samplerateIndex];

    auto config = ma_device_config_init(ma_device_type_playback);
    config.playback.pDeviceID = mMiniaudio.deviceId(soundConfig.deviceIndex);
    // always 16-bit stereo format
    config.playback.format = ma_format_s16;
    config.playback.channels = 2;
    config.performanceProfile = soundConfig.lowLatency ? ma_performance_profile_low_latency : ma_performance_profile_conservative;
    config.sampleRate = SAMPLERATE;
    config.dataCallback = audioThreadRun;
    config.pUserData = this;

    // initialize device with settings
    mDevice.emplace();
    auto err = ma_device_init(mMiniaudio.context(), &config, &mDevice.value());
    assert(err == MA_SUCCESS);
    
    mReturnBuffer.init(SAMPLERATE);

    // update the synthesizer
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
        ma_device_uninit(&mDevice.value());
    }
}

void Renderer::beginRender() {

    if (mRunning) {
        // reset the stop counter if we are already rendering
        mCancelStop = true;
    } else {

        mCallbackState = CallbackState::running;
        mBuffer.reset();
        mSamplesElapsed = 0;
        mStopCounter = 0;

        mRunning = true;
        
        mIdleCondition.wakeOne();
    }
}

void Renderer::playMusic(uint8_t orderNo, uint8_t rowNo) {
    mSpinlock.lock();
    mEngine.play(*mSongModel.currentSong(), orderNo, rowNo);
    mSpinlock.unlock();

    beginRender();
}

// SLOTS

void Renderer::clearDiagnostics() {
    mLockFails = 0;
    mUnderruns = 0;
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

    beginRender();
}

void Renderer::previewWaveform(trackerboy::Note note) {

    // state changes
    // instrument -> none -> waveform

    mSpinlock.lock();
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

    beginRender();
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
        // 1. a render has been started, begin synthesis and start the audio callback
        // 2. the Renderer is being destroyed and we must exit the loop
        mIdleCondition.wait(&mMutex);

        // [1]
        if (mRunning) {
            ma_device_start(&mDevice.value());
            emit audioStarted();
            mMutex.unlock();

            
            mCallbackMutex.lock();
            do {
                mCallbackCondition.wait(&mCallbackMutex);
            } while (!mStopDevice);
            mCallbackMutex.unlock();

            mMutex.lock();
            mStopDevice = false;
            ma_device_stop(&mDevice.value());
            mRunning = false;
            mEngine.reset();
            emit audioStopped();

        }

        // [2]
        if (mStopBackground) {
            break;
        }

    }

    mMutex.unlock();
}


constexpr int STOP_FRAMES = 5;

// ~~~~~~ CALLBACK THREAD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// called when audio data is needed to be played out to speakers. All rendering
// is done here.

void Renderer::audioThreadRun(
    ma_device *device,
    void *out,
    const void *in,
    ma_uint32 frames
) {
    (void)in;
    static_cast<Renderer*>(device->pUserData)->handleAudio(
        reinterpret_cast<int16_t*>(out),
        frames
    );
}

void Renderer::handleAudio(int16_t *out, size_t frames) {
    if (mCallbackState == CallbackState::stopped) {
        mCallbackMutex.lock();
        mCallbackCondition.wakeOne();
        mCallbackMutex.unlock();
    }

    // first we write to the buffer if possible
    // then we read out from the buffer to speakers
    // if the buffer is exhausted and we are not stopping = underrun!
    // an underrun can only occur if the buffer was exhausted and we could not lock the spinlock

    auto outIter = out;
    auto framesRemaining = frames;
    while (framesRemaining) {
        // check if the user immediately previewed/played music after stopping
        if (mCancelStop) {
            mStopCounter = 0;
            mCallbackState = CallbackState::running;
            mCancelStop = false;
        }

        
        if (mCallbackState == CallbackState::stopping) {
            if (mBuffer.isEmpty()) {
                // buffer is now empty, signal to the background thread that we are done

                // we can use a mutex here, as we no longer care about audio glitches
                // at this point
                mCallbackState = CallbackState::stopped;
                mCallbackMutex.lock();
                mStopDevice = true;
                mCallbackCondition.wakeOne();
                mCallbackMutex.unlock();
                break;
            }
        } else if (mBuffer.framesToQueue()) {

            // TODO: this implementation just synthesizes 1 frame, ideally
            // we will want to fill the render buffer completely, which will
            // minimize the chances of an underrun occurring due to spinlock failure.

            // there is space in the buffer, write to it
            if (mSpinlock.tryLock()) {
                RenderFrame rFrame;

                if (mStopCounter) {
                    if (--mStopCounter == 0) {
                        mCallbackState = CallbackState::stopping;
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
                // failed to lock the spinlock, the user (GUI thread) is modifying the document
                ++mLockFails;
                if (mBuffer.isEmpty()) {
                    // the buffer is empty and we cannot replenish it, an underrun will occur
                    // increment underrun counter and give up
                    ++mUnderruns;
                    break;
                }
            }
        }


        // write buffered data to the speakers
        size_t framesRead = mBuffer.read(outIter, framesRemaining);
        framesRemaining -= framesRead;
 
        //if (mBuffer.hasNewFrame()) {
        //    mBuffer.popFrame();
        //    mFrameSync = true;
        //    /*if (mFrameLock.tryLock()) {
        //        mLastFrameOut = mBuffer.popFrame();
        //        mFrameLock.unlock();
        //        mFrameSync = true;
        //    }*/
        //}

        outIter += framesRead * 2;
 
        mSamplesElapsed += (unsigned)framesRead;


    }

    // write what we sent to the speakers for visualizers
    mReturnBuffer.writer().fullWrite(out, frames);
    

}
