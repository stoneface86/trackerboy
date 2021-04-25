
#include "core/audio/Renderer.hpp"
#include "core/samplerates.hpp"

#include <QDeadlineTimer>
#include <QMutexLocker>
#include <QtDebug>

#include <algorithm>
#include <chrono>
#include <type_traits>

static auto LOG_PREFIX = "[Renderer]";

static void logMaResult(ma_result result) {
    qCritical() << LOG_PREFIX << result << ":" << ma_result_description(result);
}


Renderer::Renderer(
    Miniaudio &miniaudio
) :
    mMiniaudio(miniaudio),
    mIdleCondition(),
    mMutex(),
    mBackgroundThread(nullptr),
    mEnabled(false),
    mRunning(false),
    mStopBackground(false),
    mStopDevice(false),
    mAborted(false),
    mSync(false),
    mCancelStop(false),
    mDevice(),
    mDeviceConfig(),
    mLastDeviceError(MA_SUCCESS),
    mSynth(44100),
    mApu(mSynth.apu()),
    //mRc(mApu, document.mod().instrumentTable(), document.mod().waveformTable()),
    mEngine(mApu),
    mIp(),
    mPreviewState(PreviewState::none),
    mPreviewChannel(trackerboy::ChType::ch1),
    mCallbackState(CallbackState::stopped),
    mStopCounter(0),
    mBuffer(),
    mSampleReturnBuffer(),
    mSyncCounter(0),
    mSyncPeriod(0),
    mCurrentEngineFrame(),
    mFrameReturnBuffer(),
    mNewFrameSinceLastSync(false),
    mLockFails(0),
    mUnderruns(0),
    mSamplesElapsed(0),
    mBufferUsage(0)
{
    mBackgroundThread.reset(QThread::create(&Renderer::backgroundThreadRun, this));
    mBackgroundThread->start();

    mFrameReturnBuffer.init(4);

    mDeviceConfig = ma_device_config_init(ma_device_type_playback);
    // always 16-bit stereo format
    mDeviceConfig.playback.format = ma_format_s16;
    mDeviceConfig.playback.channels = 2;
    mDeviceConfig.dataCallback = audioThreadRun;
    mDeviceConfig.stopCallback = deviceStopHandler;
    mDeviceConfig.pUserData = this;
}

Renderer::~Renderer() {
    // stop the background thread
    mMutex.lock();
    mStopBackground = true;
    mStopDevice = true;
    mIdleCondition.wakeOne();
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
        mSamplesElapsed.load(),
        mBufferUsage.load(),
        (unsigned)mBuffer.size()
    };
}

ma_device const& Renderer::device() const{
    return mDevice.value();
}

AudioRingbuffer::Reader Renderer::returnBuffer() {
    return mSampleReturnBuffer.reader();
}

Ringbuffer<RenderFrame>::Reader Renderer::frameReturnBuffer() {
    return mFrameReturnBuffer.reader();
}

bool Renderer::isRunning() {
    QMutexLocker locker(&mMutex);
    return mRunning;
}

ma_result Renderer::lastDeviceError() {
    QMutexLocker locker(&mMutex);
    return mLastDeviceError;
}

void Renderer::setConfig(Config::Sound const &soundConfig) {
    QMutexLocker locker(&mMutex);
    closeDevice();

    auto const SAMPLERATE = SAMPLERATE_TABLE[soundConfig.samplerateIndex];

    // update device config
    mDeviceConfig.playback.pDeviceID = mMiniaudio.deviceId(soundConfig.deviceIndex);
    mDeviceConfig.periodSizeInFrames = (unsigned)(soundConfig.period * SAMPLERATE / 1000);
    mDeviceConfig.sampleRate = SAMPLERATE;

    // initialize device with settings
    mDevice.emplace();
    mLastDeviceError = ma_device_init(mMiniaudio.context(), &mDeviceConfig, &mDevice.value());
    mEnabled = mLastDeviceError == MA_SUCCESS;
    if (mEnabled) {
        mBuffer.init((size_t)(soundConfig.latency * SAMPLERATE / 1000));
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
    } else {
        qCritical() << LOG_PREFIX << "failed to initialize the configured device";
        logMaResult(mLastDeviceError);
    }

}

void Renderer::closeDevice() {

    if (mDevice) {
        ma_device_uninit(&mDevice.value());
        mDevice.reset();
    }
}

void Renderer::beginRender() {
    if (mRunning) {
        // reset the stop counter if we are already rendering
        mCancelStop = true;
    } else {

        mCallbackState = CallbackState::running;
        mSyncCounter = 0;
        mSync = false;
        mSamplesElapsed = 0;
        mStopCounter = 0;

        mRunning = true;
        
        mIdleCondition.wakeOne();
    }
}

void Renderer::playMusic(uint8_t orderNo, uint8_t rowNo) {

    /*mSpinlock.lock();
    mEngine.play(orderNo, rowNo);
    mSpinlock.unlock();

    QMutexLocker locker(&mMutex);
    beginRender();*/
}

// SLOTS

void Renderer::clearDiagnostics() {
    mLockFails = 0;
    mUnderruns = 0;
}

void Renderer::play() {
    //if (mEnabled) {
    //    playMusic(mSongModel.orderModel().currentPattern(), 0);
    //}
}

void Renderer::playPattern() {
    // TODO: Engine needs functionality for looping a single pattern
}

void Renderer::playFromCursor() {
    // TODO: we need a way to get the cursor row from the PatternEditor
}

void Renderer::playFromStart() {
    if (mEnabled) {
        playMusic(0, 0);
    }
}


void Renderer::previewInstrument(trackerboy::Note note) {
    //if (mEnabled) {
    //    mSpinlock.lock();
    //    switch (mPreviewState) {
    //        case PreviewState::waveform:
    //            resetPreview();
    //            [[fallthrough]];
    //        case PreviewState::none:
    //        {
    //            // set instrument runtime's instrument to the current one
    //            auto inst = mInstrumentModel.instrument(mInstrumentModel.currentIndex());
    //            mIr.setInstrument(*inst);
    //            mPreviewState = PreviewState::instrument;
    //            mPreviewChannel = static_cast<trackerboy::ChType>(inst->data().channel);
    //        }
    //        // unlock the channel for preview
    //        mEngine.unlock(mPreviewChannel);
    //        [[fallthrough]];
    //        case PreviewState::instrument:
    //            // update the current note
    //            mIr.playNote(note);
    //            break;
    //    }
    //    mSpinlock.unlock();

    //    beginRender();
    //}
}

void Renderer::previewWaveform(trackerboy::Note note) {

    //if (mEnabled) {
    //    // state changes
    //    // instrument -> none -> waveform

    //    mSpinlock.lock();
    //    switch (mPreviewState) {
    //        case PreviewState::instrument:
    //            resetPreview();
    //            [[fallthrough]];
    //        case PreviewState::none:
    //            mPreviewState = PreviewState::waveform;
    //            mPreviewChannel = trackerboy::ChType::ch3;
    //            // unlock the channel, no longer effected by music
    //            mEngine.unlock(trackerboy::ChType::ch3);
    //            // middle panning for CH3
    //            trackerboy::ChannelControl::writePanning(trackerboy::ChType::ch3, mRc, 0x11);
    //            // set the waveram with the waveform we are previewing
    //            trackerboy::ChannelControl::writeWaveram(mRc, *(mWaveModel.currentWaveform()));
    //            // volume = 100%
    //            mRc.apu.writeRegister(gbapu::Apu::REG_NR32, 0x20);
    //            // retrigger
    //            mRc.apu.writeRegister(gbapu::Apu::REG_NR34, 0x80);

    //            [[fallthrough]];
    //        case PreviewState::waveform:
    //            if (note > trackerboy::NOTE_LAST) {
    //                // should never happen, but clamp just in case
    //                note = trackerboy::NOTE_LAST;
    //            }
    //            trackerboy::ChannelControl::writeFrequency(
    //                trackerboy::ChType::ch3,
    //                mRc,
    //                trackerboy::NOTE_FREQ_TABLE[note]
    //            );
    //            break;
    //    }
    //    mSpinlock.unlock();

    //    beginRender();
    //}
}

void Renderer::stopPreview() {
    if (mEnabled) {
        /*mSpinlock.lock();
        if (mPreviewState != PreviewState::none) {
            resetPreview();
        }
        mSpinlock.unlock();*/
    }
    
}

void Renderer::stopMusic() {
    if (mEnabled) {
        /*mSpinlock.lock();
        mEngine.halt();
        mSpinlock.unlock();*/
    }
    
}

void Renderer::resetPreview() {
    // lock the channel so it can be used for music
    mEngine.lock(mPreviewChannel);
    mPreviewState = PreviewState::none;
}

// ~~~~~~ BACKGROUND THREAD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// This thread runs alongside the callback thread, for synchronizing audio with
// the GUI and stopping the callback thread when done rendering (the callback thread
// cannot stop itself).

void Renderer::backgroundThreadRun(Renderer *renderer) {
    qDebug() << LOG_PREFIX << "background thread started";
    renderer->handleBackground();
    qDebug() << LOG_PREFIX << "background thread stopped";
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
            // start the device
            mLastDeviceError = ma_device_start(&mDevice.value());
            if (mLastDeviceError == MA_SUCCESS) {

                emit audioStarted();

                do {
                    if (mSync) {
                        mSync = false;
                        emit audioSync();
                    }

                    // wait for the callback to finish playing or poll for a sync event
                    mIdleCondition.wait(&mMutex, QDeadlineTimer(1, Qt::PreciseTimer));
                } while (!mStopDevice);

                if (mAborted) {
                    mAborted = false;
                    mEnabled = false;
                    qCritical() << LOG_PREFIX << "render aborted due to device error during render";
                    emit audioError();
                } else {
                    mMutex.unlock();
                    ma_device_stop(&mDevice.value());
                    mMutex.lock();
                }
                emit audioStopped();
                mStopDevice = false;
                mEngine.reset();
            } else {
                // could not open the device, disable renderer
                mEnabled = false;
                qCritical() << LOG_PREFIX << "failed to start device";
                logMaResult(mLastDeviceError);
                emit audioError();
            }
            
            mRunning = false;
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

void Renderer::deviceStopHandler(ma_device *device) {
    // this handler is called explicitly via ma_device_stop or
    // implicitly via error during playback.
    
    // do nothing when called explicitly (mStopDevice is true)
    // force the background thread to stop when called implicitly

    Renderer *_this = static_cast<Renderer*>(device->pUserData);
    QMutexLocker locker(&_this->mMutex);
    if (!_this->mStopDevice) {
        // an error has occurred during playback, wake up the background thread
        _this->mAborted = true;
        _this->mStopDevice = true;
        _this->mIdleCondition.wakeOne();
    }
}

void Renderer::handleAudio(int16_t *out, size_t frames) {

    #if 0
    if (mCallbackState == CallbackState::stopped) {
        // do nothing, the background thread will stop the callback eventually
        return;
    }

    // Notes on the internal buffer:
    // The internal sample buffer is used to prevent underruns that can occur if we are unable
    // to lock the spinlock. If we cannot lock the spinlock via tryLock(), then the callback
    // can use the buffer instead of rendering samples. The size of the buffer is determined
    // by the latency setting and determines how many consecutive failures before an underrun occurs.
    // For example, if the period is 10ms and the latency is 30ms, we have roughly 3 attempts before
    // an underrun will occur. This is not guarenteed, as the callback function does not always
    // request an exact period every time. Latency settings should be >= period in order to prevent underruns.
    // I recommend using a latency setting 3 times that of your period setting

    // NOTE: the spinlock fails to lock only when the user is editing the document!
    // if the user is just previewing an instrument or just playing a song, no underruns will
    // occur other than from hardware issues. Underruns can only occur when the user modifies
    // the module while it is being rendered.

    
    // flag determines if the spinlock is locked. The lock is held for the entire callback
    // and is only locked when needed.
    bool locked = false;

    // internal buffer reader + writer objects
    auto reader = mBuffer.reader();
    auto writer = mBuffer.writer();

    auto outIter = out;
    auto framesRemaining = frames;
    while (framesRemaining) {
        // check if the user immediately previewed/played music after stopping
        if (mCancelStop) {
            mStopCounter = 0;
            mCallbackState = CallbackState::running;
            mCancelStop = false;
        }

        // copy from the internal buffer to the device buffer
        auto framesRead = reader.fullRead(outIter, framesRemaining);
        framesRemaining -= framesRead;
        outIter += framesRead * 2;

        

        // are we stopping?
        if (mCallbackState == CallbackState::stopping) {
            // if the buffer is now empty, signal to the background thread that we are done
            if (reader.availableRead() == 0) {
                // we can use a mutex here, as we no longer care about audio glitches
                // at this point
                mCallbackState = CallbackState::stopped;
                mMutex.lock();
                mStopDevice = true;
                mIdleCondition.wakeOne();
                mMutex.unlock();
                break;
            }
            // do nothing otherwise (let the buffer drain)
        } else {

            // replenish the internal buffer
            // (we do this last so the buffer is always full when leaving the callback)

            // attempt to acquire the lock if we have not already
            if (locked) {
                render(writer);
            } else {
                locked = mSpinlock.tryLock();
                if (locked) {
                    render(writer);
                } else {
                    // failed to acquire the lock, increment counter for diagnostics
                    ++mLockFails;
                    if (reader.availableRead() == 0 && framesRemaining) {
                        // failure!
                        // we could not acquire the lock and the buffer is empty
                        // underrun will occur
                        ++mUnderruns;
                        break;
                    }

                    // we can keep trying to lock if the buffer is not empty
                }
            }
        }
    }

    // unlock the spinlock if it was locked
    if (locked) {
        mSpinlock.unlock();
    }

    // update sync counter, check if we have outputted an entire period
    mSyncCounter += frames;
    if (mSyncCounter >= mSyncPeriod) {

        auto frameWriter = mFrameReturnBuffer.writer();
        if (mNewFrameSinceLastSync && frameWriter.availableWrite()) {

            size_t toWrite = 1;
            auto rframe = frameWriter.acquireWrite(toWrite);

            rframe->engineFrame = mCurrentEngineFrame;
            rframe->registers = mSynth.apu().registers();

            frameWriter.commitWrite(rframe, toWrite);


            mNewFrameSinceLastSync = false;
        }

        mSyncCounter %= mSyncPeriod;
        mSync = true;
    }

    // write what we sent to the device to the return buffer
    // the GUI will read this on sync event for visualizers
    mSampleReturnBuffer.writer().fullWrite(out, frames);
    
    // update diagnostic counters
    mSamplesElapsed += (unsigned)frames;
    mBufferUsage = (unsigned)reader.availableRead();
    #endif
}

void Renderer::render(AudioRingbuffer::Writer &writer) {
    auto &apu = mSynth.apu();
    auto samplesToRender = writer.availableWrite();
    while (samplesToRender) {

        if (apu.availableSamples() == 0) {
            if (mCallbackState == CallbackState::stopping) {
                break; // no more rendering at this point
            }

            if (mStopCounter) {
                if (--mStopCounter == 0) {
                    mCallbackState = CallbackState::stopping;
                }
            } else {
                mNewFrameSinceLastSync = true;
                
                // step the engine
                trackerboy::Engine::Frame frame;
                mEngine.step(frame);
                mCurrentEngineFrame = frame;

                // step the instrument runtime if we are previewing an instrument
                //if (mPreviewState == PreviewState::instrument) {
                //    mIr.step();
                //}

                // begin the stop countdown if the engine halted and we are not previewing anything
                if (frame.halted && mPreviewState == PreviewState::none) {
                    mStopCounter = STOP_FRAMES;
                }

            }

            // synthesize the frame
            mSynth.run();
        }

        // write the synth buffer to the internal buffer
        size_t toWrite = samplesToRender;
        auto out = writer.acquireWrite(toWrite);
        apu.readSamples(out, toWrite);
        writer.commitWrite(out, toWrite);
        
        // update loop counter
        samplesToRender -= toWrite;


    }
}
