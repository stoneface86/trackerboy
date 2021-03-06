
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
    //mCallbackCondition(),
    mMutex(),
    //mCallbackMutex(),
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
    //mBuffer(),
    //mReturnBuffer(),
    mFrameBuffer(nullptr),
    mFrameBuffersize(0),
    mSyncCounter(0),
    mSyncPeriod(0),
    mCurrentFrameLock(),
    mNewFrameSinceLastSync(false),
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
    //mCallbackMutex.lock();

    mStopBackground = true;
    mStopDevice = true;
    mIdleCondition.wakeOne();
    //mCallbackCondition.wakeOne();

    //mCallbackMutex.unlock();
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
    return mSampleReturnBuffer.reader();
}

//Ringbuffer<RenderFrame>::Reader Renderer::frameReturnBuffer() {
//    return mFrameReturnBuffer.reader();
//}

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
    config.periodSizeInFrames = (unsigned)(soundConfig.period * SAMPLERATE / 1000);
    config.sampleRate = SAMPLERATE;
    config.dataCallback = audioThreadRun;
    config.pUserData = this;

    // initialize device with settings
    mDevice.emplace();
    auto err = ma_device_init(mMiniaudio.context(), &config, &mDevice.value());
    // TODO: handle error conditions
    assert(err == MA_SUCCESS);
    
    //mBuffer.init(soundConfig.latency * SAMPLERATE / 1000);
    mSampleReturnBuffer.init(SAMPLERATE);

    mSyncPeriod = mDevice.value().playback.internalPeriodSizeInFrames;

    // update the synthesizer
    mSynth.setSamplingRate(SAMPLERATE);
    mSynth.apu().setQuality(static_cast<gbapu::Apu::Quality>(soundConfig.quality));
    mSynth.setupBuffers();

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
        mFrameBuffersize = 0;
        mSyncCounter = 0;
        mSync = false;
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
            // start the device (TODO: error check!)
            ma_device_start(&mDevice.value());
            emit audioStarted();
            
            //mMutex.unlock();

            do {
                if (mSync) {
                    mSync = false;
                    emit audioSync();
                }

                // wait for the callback to finish playing or poll for a sync event
                mIdleCondition.wait(&mMutex, QDeadlineTimer(1, Qt::PreciseTimer));
            } while (!mStopDevice);

            //mMutex.lock();

            // stop the device
            ma_device_stop(&mDevice.value());
            emit audioStopped();
            mStopDevice = false;
            mRunning = false;
            mEngine.reset();

        }

        // [2]
        if (mStopBackground) {
            break;
        }

    }

    mMutex.unlock();
}

// this is the number of frames to output before stopping playback
// (prevents a hard pop noise that may occur when stopping abruptly, as
// the high pass filter will decay the signal to 0)
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
        // do nothing, the background thread will stop the callback eventually
        return;
    }

    // first we write to the buffer if possible
    // then we read out from the buffer to speakers
    // if the buffer is exhausted and we are not stopping = underrun!
    // an underrun can only occur if the buffer was exhausted and we could not lock the spinlock

    bool locked = false;

    // internal buffer reader + writer objects
    //auto reader = mBuffer.reader();
    //auto writer = mBuffer.writer();

    // frame and sample return buffer writers (to send to the GUI)
    //auto frameWriter = mFrameReturnBuffer.writer();
    auto sampleReturnWriter = mSampleReturnBuffer.writer();

    auto outIter = out;
    auto framesRemaining = frames;
    while (framesRemaining) {
        // check if the user immediately previewed/played music after stopping
        if (mCancelStop) {
            mStopCounter = 0;
            mCallbackState = CallbackState::running;
            mCancelStop = false;
        }

        if (mFrameBuffersize == 0) {
            if (mCallbackState == CallbackState::stopping) {
                // buffer is now empty, signal to the background thread that we are done

                // we can use a mutex here, as we no longer care about audio glitches
                // at this point
                mCallbackState = CallbackState::stopped;
                mMutex.lock();
                mStopDevice = true;
                mIdleCondition.wakeOne();
                mMutex.unlock();
                break;
            } else {

                // attempt to acquire the lock if we have not already
                if (!locked) {
                    locked = mSpinlock.tryLock();
                    if (!locked) {
                        ++mLockFails;
                        break;
                    }
                }

                if (mStopCounter) {
                    if (--mStopCounter == 0) {
                        mCallbackState = CallbackState::stopping;
                    }
                } else {
                    mNewFrameSinceLastSync = true;
                    //RenderFrame rFrame;
                    // step the engine
                    mEngine.step(mCurrentFrame.engineFrame);

                    // step the instrument runtime if we are previewing an instrument
                    if (mPreviewState == PreviewState::instrument) {
                        mIr.step();
                    }

                    // begin the stop countdown if the engine halted and we are not previewing anything
                    if (mCurrentFrame.engineFrame.halted && mPreviewState == PreviewState::none) {
                        mStopCounter = STOP_FRAMES;
                    }

                    mCurrentFrame.registers = mSynth.apu().registers();
                    //frameWriter.fullWrite(&rFrame, 1);
                }

                // get the current register dump

                // synthesize the frame
                mFrameBuffersize = mSynth.run();
                mFrameBuffer = mSynth.buffer();
            }
        }


        // write buffered data to the speakers
        auto toWrite = std::min(mFrameBuffersize, framesRemaining);
        auto toWriteInSamples = toWrite * 2;

        // copy from frame buffer to out
        std::copy_n(mFrameBuffer, toWriteInSamples, outIter);
        mFrameBuffersize -= toWrite;
        mFrameBuffer += toWriteInSamples;
        
        // adjust out iterator
        framesRemaining -= toWrite;
        outIter += toWriteInSamples;
 
        


    }

    if (locked) {
        mSpinlock.unlock();
    }

    mSyncCounter += frames;
    if (mSyncCounter >= mSyncPeriod) {
        if (mNewFrameSinceLastSync && mCurrentFrameLock.tryLock()) {
            // Save a copy for the GUI
            mCurrentFrameCopy = mCurrentFrame;
            mNewFrameSinceLastSync = false;
            mCurrentFrameLock.unlock();
        }

        mSyncCounter %= mSyncPeriod;
        mSync = true;
    }

    sampleReturnWriter.fullWrite(out, frames);
    mSamplesElapsed += (unsigned)frames;

}

