
#include "core/audio/Renderer.hpp"
#include "core/samplerates.hpp"

#include <QMutexLocker>
#include <QtDebug>

#include <algorithm>
#include <type_traits>

Renderer::Renderer(
    Miniaudio &miniaudio,
    ModuleDocument &document,
    InstrumentListModel &instrumentModel,
    SongListModel &songModel,
    WaveListModel &waveModel
) :
    mMiniaudio(miniaudio),
    mDocument(document),
    mInstrumentModel(instrumentModel),
    mSongModel(songModel),
    mWaveModel(waveModel),
    mIdleCondition(),
    mMutex(),
    mBackgroundThread(nullptr),
    mRunning(false),
    mStopBackground(false),
    mStopDevice(false),
    mDevice(nullptr),
    mSynth(44100),
    mRc(mSynth.apu(), document.instrumentTable(), document.waveTable()),
    mEngine(mRc),
    mIr(mRc),
    mPreviewState(PreviewState::none),
    mPreviewChannel(trackerboy::ChType::ch1),
    mSynthBufferRemaining(0),
    mSynthBuffer(nullptr),
    mStopCounter(0)
{
    mBackgroundThread.reset(QThread::create(&Renderer::backgroundThreadRun, this));
    mBackgroundThread->start();
}

Renderer::~Renderer() {
    // stop the background thread
    mMutex.lock();
    
    mStopBackground = true;
    mStopDevice = true;
    // stop the sync
    mStream.interrupt();
    mIdleCondition.wakeOne();
    
    mMutex.unlock();


    // wake and wait for the background thread to finish
    mBackgroundThread->wait();

    closeDevice();
}

Renderer::Diagnostics Renderer::diagnostics() {
    size_t bufferSize = mStream.bufferSize();
    return {
        mStream.underruns(),
        mStream.elapsed(),
        bufferSize - mStream.buffer().availableWrite(),
        bufferSize,
        mStream.syncTime()
    };
}

ma_device const& Renderer::device() const{
    return *mDevice.get();
}

bool Renderer::isRunning() {
    QMutexLocker locker(&mMutex);
    return mRunning;
}

void Renderer::setConfig(Config::Sound const &soundConfig) {
    QMutexLocker locker(&mMutex);
    

    auto const SAMPLERATE = SAMPLERATE_TABLE[soundConfig.samplerateIndex];

    auto config = ma_device_config_init(ma_device_type_playback);
    config.playback.pDeviceID = mMiniaudio.deviceId(soundConfig.deviceIndex);
    // always 16-bit stereo format
    config.playback.format = ma_format_s16;
    config.playback.channels = 2;
    config.performanceProfile = soundConfig.lowLatency ? ma_performance_profile_low_latency : ma_performance_profile_conservative;
    config.sampleRate = SAMPLERATE;
    config.dataCallback = AudioOutStream::callback;
    config.pUserData = &mStream;

    // initialize device with settings
    std::unique_ptr<ma_device> device(new ma_device);
    auto err = ma_device_init(mMiniaudio.context(), &config, device.get());
    assert(err == MA_SUCCESS);

    // stream will use the new device
    mStream.setDevice(device.get());
    // close the old device
    closeDevice();
    // the old device can now be deleted and replaced by the new one
    mDevice = std::move(device);

    // update the synthesizer
    mSynth.setSamplingRate(SAMPLERATE);
    mSynth.setVolume(soundConfig.volume);
    mSynth.setupBuffers();

    // if we were running beforehand anything in the buffer was lost so we must replensish it
    // jank fix this later (glitches when user changes the sample rate)
    if (mRunning && !mStopDevice) {
        auto buffer = mStream.buffer();
        render(buffer);
    }

}

void Renderer::closeDevice() {

    if (mDevice) {
        ma_device_uninit(mDevice.get());
    }
}

void Renderer::beginRender() {

    // reset the stop counter if we are already rendering
    mStopCounter = 0;
    mStopDevice = false;

    if (!mRunning) {

        mSynthBufferRemaining = 0;

        mRunning = true;
        mIdleCondition.wakeOne();
    }
}

void Renderer::playMusic(uint8_t orderNo, uint8_t rowNo) {
    mMutex.lock();
    mEngine.play(*mSongModel.currentSong(), orderNo, rowNo);
    mMutex.unlock();

    beginRender();
}

// SLOTS

void Renderer::clearDiagnostics() {
    mStream.resetUnderruns();
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
    QMutexLocker locker(&mMutex);
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

    beginRender();
}

void Renderer::previewWaveform(trackerboy::Note note) {
    QMutexLocker locker(&mMutex);

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

    beginRender();
}

void Renderer::stopPreview() {
    QMutexLocker locker(&mMutex);

    if (mPreviewState != PreviewState::none) {
        resetPreview();
    }
    
}

void Renderer::stopMusic() {
    QMutexLocker locker(&mMutex);
    mEngine.halt();
    
}



void Renderer::resetPreview() {
    // lock the channel so it can be used for music
    trackerboy::ChannelControl::writePanning(mPreviewChannel, mRc, 0);
    mEngine.lock(mPreviewChannel);
    mPreviewState = PreviewState::none;
}

// ~~~~~~ BACKGROUND THREAD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// This thread handles all renderering and sends the synthesized audio to the
// speakers. When there is nothing to render, the thread idles via mIdleCondition.

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

            // fill the buffer before starting the stream
            auto buffer = mStream.buffer();
            render(buffer);
            
            auto error = mStream.start();
            if (error == MA_SUCCESS) {
                //emit audioStarted();

                mStream.setIdle(false);
                for (;;) {
                    // synchronize with the stream
                    mMutex.unlock();
                    mStream.sync();
                    mMutex.lock();

                    if (mStopDevice) {
                        break;
                    }


                    // keep rendering to the buffer
                    render(buffer);


                }
                mStream.setIdle(true);

                mStopDevice = false;
                mRunning = false;
                mEngine.reset();
                mStream.stop();
                //emit audioStopped();
            } else {
                qDebug() << "Failed to start device";
                //emit audioError();
            }

        }

        // [2]
        if (mStopBackground) {
            break;
        }

    }

    mMutex.unlock();
}


constexpr int STOP_FRAMES = 5;

void Renderer::render(AudioRingbuffer::Writer &buffer) {

    bool newFrame = false;
    auto nsamples = buffer.availableWrite();
    while (nsamples) {
        if (mSynthBufferRemaining == 0) {

            // render a new frame
            newFrame = true;

            mDocument.lock();

            if (mStopCounter) {
                if (--mStopCounter == 0) {
                    mDocument.unlock();
                    mStopDevice = true;
                    break;
                        
                }
            } else {
                trackerboy::Frame f;
                mEngine.step(f);
                // step the instrument runtime if we are previewing an instrument
                if (mPreviewState == PreviewState::instrument) {
                    mIr.step();
                }

                // begin the stop countdown if the engine halted and we are not previewing anything
                if (f.halted && mPreviewState == PreviewState::none) {
                    mStopCounter = STOP_FRAMES;
                }
            }

            // synthesize the frame
            mSynthBufferRemaining = mSynth.run();
            mSynthBuffer = mSynth.buffer();

            mDocument.unlock();
            

        }

        auto toWrite = std::min(mSynthBufferRemaining, nsamples);
        auto written = buffer.fullWrite(mSynthBuffer, toWrite);
        nsamples -= written;
        mSynthBufferRemaining -= written;
        mSynthBuffer += written * 2;


    }

    if (newFrame) {
        emit frameSync();
    }
}


//void Renderer::handleCallback(int16_t *out, size_t frames) {
//
//    if (mStopped) {
//        // keep waking the background thread in case of lost wakeup
//        mCallbackCondition.wakeOne();
//        return;
//    }
//
//    mBufferUsage = mBuffer.framesInQueue();
//
//    while (frames) {
//
//        // check if the user immediately previewed/played music after stopping
//        if (mCancelStop) {
//            mStopCounter = 0;
//            mShouldStop = false;
//            mCancelStop = false;
//        }
//
//        if (mShouldStop) {
//            // do not render anything if we should stop
//            if (mBuffer.isEmpty()) {
//                // buffer is now empty, signal to the background thread that we are done
//
//                // we can use a mutex here, as we no longer care about audio glitches
//                // at this point
//                mCallbackMutex.lock();
//                mStopDevice = true;
//                mCallbackMutex.unlock();
//
//                mStopped = true;
//                mCallbackCondition.wakeOne();
//                return;
//            }
//        } else if (mBuffer.framesToQueue()) {
//            // attempt to generate a frame and queue it for playback
//
//            if (mSpinlock.tryLock()) {
//
//                RenderFrame rFrame;
//
//                if (mStopCounter) {
//                    if (--mStopCounter == 0) {
//                        mShouldStop = true;
//                    }
//                    // the GUI should ignore this frame on audio sync event
//                    rFrame.ignore = true;
//                } else {
//
//                    // step the engine
//                    mEngine.step(rFrame.engineFrame);
//
//                    // step the instrument runtime if we are previewing an instrument
//                    if (mPreviewState == PreviewState::instrument) {
//                        mIr.step();
//                    }
//
//                    // begin the stop countdown if the engine halted and we are not previewing anything
//                    if (rFrame.engineFrame.halted && mPreviewState == PreviewState::none) {
//                        mStopCounter = STOP_FRAMES;
//                    }
//
//                    // do not ignore this frame
//                    rFrame.ignore = false;
//                }
//
//                // get the current register dump
//                rFrame.registers = mSynth.apu().registers();
//
//                // synthesize the frame
//                rFrame.nsamples = mSynth.run();
//
//                // queue it for future playback
//                mBuffer.queueFrame(mSynth.buffer(), rFrame);
//
//                mSpinlock.unlock();
//            } else {
//                // failed to lock spinlock, do nothing and increment counter for diagnostics
//                ++mLockFails;
//                if (mBuffer.isEmpty()) {
//                    // give up, underrun will occur
//                    ++mUnderruns;
//                    break;
//                }
//            }
//        }
//
//        // write buffered data to the speakers
//        size_t framesRead = mBuffer.read(out, frames);
//        frames -= framesRead;
//
//        if (mBuffer.hasNewFrame()) {
//            if (mFrameLock.tryLock()) {
//                mLastFrameOut = mBuffer.popFrame();
//                mFrameLock.unlock();
//                
//            }
//        }
//
//        // write what we sent to the speakers for visualizers
//        mReturnBuffer.fullWrite(out, framesRead);
//
//        auto samplesRead = framesRead * 2;
//        out += samplesRead;
//        mSyncCounter += framesRead;
//
//        mSamplesElapsed += (unsigned)framesRead;
//    }
//
//    if (mSyncCounter >= mSyncPeriod) {
//        // sync audio when we have written at least mSyncPeriod samples
//        mSync = true;
//        #ifndef USE_POLLING
//        mCallbackCondition.wakeOne();
//        #endif
//        mSyncCounter %= mSyncPeriod;
//    }
//}
