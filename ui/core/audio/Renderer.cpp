
#include "core/audio/Renderer.hpp"
#include "core/samplerates.hpp"

#include "trackerboy/engine/ChannelControl.hpp"

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


Renderer::Renderer(QObject *parent) :
    QObject(parent),
    mMutex(),
    mDocument(nullptr),
    mDeviceConfig(),
    mDevice(),
    mBuffer(),
    mLastDeviceError(MA_SUCCESS),
    mEnabled(false),
    mSynth(44100),
    mApu(mSynth.apu()),
    mEngine(mApu, nullptr),
    mIp(),
    mPreviewState(PreviewState::none),
    mPreviewChannel(trackerboy::ChType::ch1),
    mState(State::stopped),
    mStopCounter(0),
    mDraining(false),
    mUnderruns(0u),
    mSamplesElapsed(0u),
    mBufferUsage(0u)
{
    // mBackgroundThread.reset(QThread::create(&Renderer::backgroundThreadRun, this));
    // mBackgroundThread->start();

    // mFrameReturnBuffer.init(4);

    mDeviceConfig = ma_device_config_init(ma_device_type_playback);
    // always 16-bit stereo format
    mDeviceConfig.playback.format = ma_format_s16;
    mDeviceConfig.playback.channels = 2;
    mDeviceConfig.dataCallback = deviceDataHandler;
    mDeviceConfig.stopCallback = deviceStopHandler;
    mDeviceConfig.pUserData = this;
}

Renderer::~Renderer() {
    closeDevice();
}

Renderer::Diagnostics Renderer::diagnostics() {
    QMutexLocker locker(&mMutex);
    return {
        0,
        mUnderruns.load(),
        mSamplesElapsed.load(),
        mBufferUsage.load(),
        (unsigned)mBuffer.size()
    };
}

ma_device const& Renderer::device() {
    QMutexLocker locker(&mMutex);
    return *mDevice;
}

ModuleDocument* Renderer::document() {
    QMutexLocker locker(&mMutex);
    return mDocument;
}

// AudioRingbuffer::Reader Renderer::returnBuffer() {
//     return mSampleReturnBuffer.reader();
// }

// Ringbuffer<RenderFrame>::Reader Renderer::frameReturnBuffer() {
//     return mFrameReturnBuffer.reader();
// }

bool Renderer::isRunning() {
    QMutexLocker locker(&mMutex);
    return mState != State::stopped;
}

bool Renderer::isEnabled() {
    QMutexLocker locker(&mMutex);
    return mEnabled;
}

ma_result Renderer::lastDeviceError() {
    QMutexLocker locker(&mMutex);
    return mLastDeviceError;
}

void Renderer::setConfig(Miniaudio &miniaudio, Config::Sound const &soundConfig) {
    QMutexLocker locker(&mMutex);
    closeDevice();

    auto const SAMPLERATE = SAMPLERATE_TABLE[soundConfig.samplerateIndex];

    // update device config
    mDeviceConfig.playback.pDeviceID = miniaudio.deviceId(soundConfig.deviceIndex);
    mDeviceConfig.periodSizeInFrames = (unsigned)(soundConfig.period * SAMPLERATE / 1000);
    mDeviceConfig.sampleRate = SAMPLERATE;

    // initialize device with settings
    mDevice.emplace();
    mLastDeviceError = ma_device_init(miniaudio.context(), &mDeviceConfig, &mDevice.value());
    mEnabled = mLastDeviceError == MA_SUCCESS;
    if (mEnabled) {
        mBuffer.init((size_t)(soundConfig.latency * SAMPLERATE / 1000));
        //mSampleReturnBuffer.init(SAMPLERATE);

        //mSyncPeriod = mDevice.value().playback.internalPeriodSizeInFrames;

        // update the synthesizer
        mSynth.setSamplingRate(SAMPLERATE);
        mSynth.apu().setQuality(static_cast<gbapu::Apu::Quality>(soundConfig.quality));
        mSynth.setupBuffers();

        if (mState != State::stopped) {
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
    if (mState == State::stopped) {
        mBuffer.reset();
        // auto writer = mBuffer.writer();
        // size_t toWrite = writer.availableWrite();
        // auto dataPtr = writer.acquireWrite(toWrite);
        // std::fill_n(dataPtr, toWrite * 2, (int16_t)0);
        // writer.commitWrite(dataPtr, toWrite);
        mLastDeviceError = ma_device_start(&*mDevice);
        if (mLastDeviceError != MA_SUCCESS) {
            emit audioError();
            return;
        }
        emit audioStarted();
        mSamplesElapsed = 0;
    }
    mDraining = false;
    mState = State::running;
    mStopCounter = 0;
    mMutex.unlock();
    render();
    mMutex.lock();
}



// SLOTS

void Renderer::clearDiagnostics() {
    mUnderruns = 0;
}


void Renderer::setDocument(ModuleDocument *doc) {
    mDocument = doc;
}

// void Renderer::playMusic(uint8_t orderNo, uint8_t rowNo) {

//     /*mSpinlock.lock();
//     mEngine.play(orderNo, rowNo);
//     mSpinlock.unlock();

//     QMutexLocker locker(&mMutex);
//     beginRender();*/
// }

// void Renderer::play() {
//     //if (mEnabled) {
//     //    playMusic(mSongModel.orderModel().currentPattern(), 0);
//     //}
// }

// void Renderer::playPattern() {
//     // TODO: Engine needs functionality for looping a single pattern
// }

// void Renderer::playFromCursor() {
//     // TODO: we need a way to get the cursor row from the PatternEditor
// }

// void Renderer::playFromStart() {
//     if (mEnabled) {
//         playMusic(0, 0);
//     }
// }


void Renderer::previewInstrument(quint8 note) {
    QMutexLocker locker(&mMutex);
    if (mEnabled) {
       switch (mPreviewState) {
           case PreviewState::waveform:
               resetPreview();
               [[fallthrough]];
           case PreviewState::none:
           {
               // set instrument preview's instrument to the current one
               auto inst = mDocument->instrumentModel().currentInstrument();
               mPreviewChannel = inst->channel();
               mIp.setInstrument(std::move(inst));
               mPreviewState = PreviewState::instrument;
           }
           // unlock the channel for preview
           mEngine.unlock(mPreviewChannel);
           [[fallthrough]];
           case PreviewState::instrument:
               // update the current note
               mIp.play(note);
               break;
       }

       beginRender();
    }
}

void Renderer::previewWaveform(quint8 note) {
    QMutexLocker locker(&mMutex);
    if (mEnabled) {
        // state changes
        // instrument -> none -> waveform

        if (note > trackerboy::NOTE_LAST) {
            // should never happen, but clamp just in case
            note = trackerboy::NOTE_LAST;
        }
        auto freq = trackerboy::NOTE_FREQ_TABLE[note];

        switch (mPreviewState) {
            case PreviewState::instrument:
                resetPreview();
                [[fallthrough]];
            case PreviewState::none: {
                mPreviewState = PreviewState::waveform;
                mPreviewChannel = trackerboy::ChType::ch3;
                // unlock the channel, no longer effected by music
                mEngine.unlock(trackerboy::ChType::ch3);

                trackerboy::ChannelState state(trackerboy::ChType::ch3);
                state.playing = true;
                state.envelope = mDocument->waveModel().currentWaveform()->id();
                state.frequency = freq;
                trackerboy::ChannelControl<trackerboy::ChType::ch3>::init(mApu, mDocument->mod().waveformTable(), state);
                break;
            }
            case PreviewState::waveform:
                // already previewing, just update the frequency
                mApu.writeRegister(gbapu::Apu::REG_NR33, (uint8_t)(freq & 0xFF));
                mApu.writeRegister(gbapu::Apu::REG_NR33, (uint8_t)(freq >> 8));
                break;
        }

        beginRender();
    }
}

void Renderer::stopPreview() {
    QMutexLocker locker(&mMutex);

    if (mEnabled) {
        if (mPreviewState != PreviewState::none) {
            resetPreview();
        }
    }
    
}

// void Renderer::stopMusic() {
//     if (mEnabled) {
//         /*mSpinlock.lock();
//         mEngine.halt();
//         mSpinlock.unlock();*/
//     }
    
// }

// void Renderer::stopAll() {
//     stopPreview();
//     stopMusic();
// }

void Renderer::resetPreview() {
    // lock the channel so it can be used for music
    mEngine.lock(mPreviewChannel);
    mIp.setInstrument(nullptr);
    mPreviewState = PreviewState::none;
}

// this is the number of frames to output before stopping playback
// (prevents a hard pop noise that may occur when stopping abruptly, as
// the high pass filter will decay the signal to 0)
constexpr int STOP_FRAMES = 5;

void Renderer::render() {
    QMutexLocker locker(&mMutex);

    if (mState == State::stopped) {
        return;
    }

    auto writer = mBuffer.writer();
    auto framesToRender = writer.availableWrite();

    auto &apu = mSynth.apu();

    bool newFrame = false;

    while (framesToRender) {

        if (mState == State::stopping) {
            if (writer.availableWrite() == mBuffer.size()) {
                // the buffer has been drained, stop the callback
                mState = State::stopped;
                auto device = &*mDevice;

                locker.unlock();
                ma_device_stop(device);
                locker.relock();

                emit audioStopped();
            }
            return;

        } else {

            if (apu.availableSamples() == 0) {
                // new frame

                if (mState == State::stopping) {
                    break; // stop, don't render any more
                }

                if (mStopCounter) {
                    if (--mStopCounter == 0) {
                        mState = State::stopping;
                        // no longer counting underruns
                        mDraining = true;
                    }
                } else {
                    newFrame = true;

                    // step engine/previewer
                    mEngine.step(mCurrentEngineFrame);

                    if (mPreviewState == PreviewState::instrument) {
                        auto &mod = mDocument->mod();
                        trackerboy::RuntimeContext rc(mApu, mod.instrumentTable(), mod.waveformTable());
                        mIp.step(rc);
                    }

                    if (mCurrentEngineFrame.halted && mPreviewState == PreviewState::none) {
                        // no longer doing anything, start the stop counter
                        mStopCounter = STOP_FRAMES;
                    }

                }

                mSynth.run();

            }

            size_t toWrite = std::min(framesToRender, apu.availableSamples());
            auto writePtr = writer.acquireWrite(toWrite);
            apu.readSamples(writePtr, toWrite);
            writer.commitWrite(writePtr, toWrite);
            
            framesToRender -= toWrite;

        }

    }

    if (newFrame) {
        emit frameSync();
    }

}

// ~~~~~~ CALLBACK THREAD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// called when audio data is needed to be played out to speakers.

void Renderer::deviceStopHandler(ma_device *device) {
    // this handler is called explicitly via ma_device_stop or
    // implicitly via error during playback.
    
    // do nothing when called explicitly (mStopDevice is true)
    // force the background thread to stop when called implicitly

    static_cast<Renderer*>(device->pUserData)->_deviceStopHandler();
}

void Renderer::_deviceStopHandler() {
    QMutexLocker locker(&mMutex);

    if (mState != State::stopped) {
        // we didn't stop the callback explicitly, a device error has occurred
        mState = State::stopped;
        emit audioError();
    }
}

void Renderer::deviceDataHandler(ma_device *device, void *out, const void *in, ma_uint32 frames) {
    static_cast<Renderer*>(device->pUserData)->_deviceDataHandler(
        static_cast<int16_t*>(out),
        (size_t)frames
    );
}

void Renderer::_deviceDataHandler(int16_t *out, size_t frames) {

    auto reader = mBuffer.reader();

    // update buffer utilization (for diagnostics)
    // low utilization indicates that the render thread can't keep up with
    // the callback, and underruns may occur
    mBufferUsage = (unsigned)reader.availableRead();

    // read from the buffer to the device buffer
    auto nread = reader.fullRead(out, frames);

    // if we didn't read enough and we are not draining the buffer, then
    // an underrun has occurred
    if (nread != frames && !mDraining) {
        ++mUnderruns;
    }

    // increment the total samples elapsed
    mSamplesElapsed += (unsigned)frames;
}
