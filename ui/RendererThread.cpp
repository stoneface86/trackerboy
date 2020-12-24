
#include "RendererThread.hpp"

#include "trackerboy/engine/ChannelControl.hpp"

#include <cstdint>

// TODO: the callback thread should buffer more than 1 frame. Gaps may occur if
// locking the spinlock fails when rendering the frame. More frames gives us
// more chances at the cost of latency.

// Details
//
// Rendering is done in a separate thread, known as the audio callback thread. The
// RendererThread class manages this callback. There are three types of renders,
// waveform preview, instrument preview and music playback. The preview renders
// are for testing a waveform or instrument and can be rendered alongside music.
//
// Since renderering is done exclusively in the callback thread, we need a way
// to communicate the work it needs to do from the GUI thread. This is done
// via an atomic variable, mCommand. We use atomics because we cannot use
// normal synchronization primitives as their use is not time-bounded. The
// callback function is called in real-time and must be time-bounded,
// otherwise gaps in audio playback may occur.
//
// mCommand is a 16-bit word that contains a command identifier in the upper byte
// and the argument in lower byte. See RendererThread::Command for a list of possible
// commands. The GUI thread can schedule a command via the RendererThread::command
// method. This command will be executed the next time the callback thread needs
// to synthesize a frame. 
//
// When the callback thread has work to do, it synthesizes a number of frames. The
// amount of frames it buffers can be configured in the settings (default is 1).
// The buffer is consumed as needed by the callback thread, which then plays it out
// to the speakers. When this buffer has enough room for a frame, it is synthesized
// if the callback thread can safely access the document.
//
// Synchronization
//
// Both the GUI thread and the callback thread have access to the document, or, the module.
// Both of these threads can read the document, but only the GUI thread can modify it.
// In order to ensure thread safety, mutual exclusion will be used in the form of a
// Spinlock. Reminder that a regular mutex cannot be used because a mutex is not time-bounded.
//
// When the GUI thread needs to modify the document, it will lock the spinlock and unlock it
// when finished. When the callback thread needs to read the document it will attempt to lock
// the document (trylock). If the callback thread fails to lock it and the buffer is exhausted
// before it can try again, there will be a gap in the audio playback. Keep in mind this only
// happens if the user is editing while there is rendering going on. A larger buffer will
// prevent this from occurring at the cost of higher latency.


RendererThread::RendererThread(ModuleDocument &document,
                               InstrumentListModel &instrumentModel,
                               WaveListModel &waveModel,
                               QWaitCondition &stopCondition
) :
    mStopCondition(stopCondition),
    mRunning(false),
    mCommand(0),
    mDocument(document),
    mInstrumentModel(instrumentModel),
    mWaveModel(waveModel),
    mSynth(44100),
    mRc(mSynth.apu(), document.instrumentTable(), document.waveTable()),
    mEngine(mRc),
    mIr(mRc),
    mPreviewState(PreviewState::none),
    mPreviewChannel(trackerboy::ChType::ch1),
    mStopCounter(0),
    mStopped(false),
    mMusicPlaying(false),
    mFrameBuffer(nullptr),
    mFrameRemaining(0)
{
}

RendererThread::~RendererThread() {
    close();
}

void RendererThread::applyConfig(Config::Sound const &soundConfig) {
    close();

    auto config = ma_device_config_init(ma_device_type_playback);
    if (soundConfig.device != nullptr) {
        config.playback.pDeviceID = soundConfig.device;
    }
    config.playback.format = ma_format_s16;
    config.playback.channels = 2;
    config.performanceProfile = soundConfig.lowLatency ? ma_performance_profile_low_latency : ma_performance_profile_conservative;
    config.sampleRate = soundConfig.samplerate;
    config.dataCallback = audioCallback;
    config.pUserData = this;

    mDevice.emplace();
    auto err = ma_device_init(soundConfig.context, &config, &mDevice.value());
    assert(err == MA_SUCCESS);

    mSynth.setSamplingRate(soundConfig.samplerate);
    mSynth.setVolume(soundConfig.volume);
    mSynth.setupBuffers();
}

void RendererThread::close() {
    if (mDevice) {
        ma_device_uninit(&mDevice.value());
        mDevice.reset();
    }
}

void RendererThread::command(Command cmd, uint8_t data) {
    // set the command
    uint16_t packed = (static_cast<uint16_t>(cmd) << 8) | data;
    mCommand = packed;

    mMutex.lock();
    if (!mRunning) {
        ma_device_start(&mDevice.value());
        mRunning = true;
    }
    mMutex.unlock();
}

void RendererThread::stop() {
    QMutexLocker locker(&mMutex);
    if (mRunning) {
        ma_device_stop(&mDevice.value());
        mStopped = false;
        mRunning = false;
    }
}


// ----------------------------------------------------------------------------
// Functions below this line are to be run in the callback thread


void RendererThread::audioCallback(
    ma_device *device,
    void *out, 
    const void *in,
    ma_uint32 frames
) {
    (void)in;
    static_cast<RendererThread*>(device->pUserData)->handleCallback(
        reinterpret_cast<int16_t*>(out),
        frames
    );
}

// number of frames to play out when stopping (so the high pass filter decays the signal to 0)
constexpr int STOP_FRAMES = 2;

void RendererThread::handleCallback(int16_t *out, size_t frames) {

    if (mStopped) {
        // keep waking the background thread
        mStopCondition.wakeOne();
        return;
    }

    while (frames) {
        if (mFrameRemaining == 0) {
            // process new frame

            if (!mDocument.trylock()) {
                return;
            }

            uint16_t command = mCommand.exchange(0);
            uint8_t arg = command & 0xFF;
            switch (static_cast<Command>(command >> 8)) {
                case Command::none:
                    break;
                case Command::previewWave:
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
                            if (arg > trackerboy::NOTE_LAST) {
                                // should never happen, but clamp just in case
                                arg = trackerboy::NOTE_LAST;
                            }
                            trackerboy::ChannelControl::writeFrequency(
                                trackerboy::ChType::ch3,
                                mRc,
                                trackerboy::NOTE_FREQ_TABLE[arg]
                            );
                            break;
                    }
                    break;
                case Command::previewInst:
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
                        mIr.playNote(static_cast<trackerboy::Note>(arg));

                        [[fallthrough]];
                        case PreviewState::instrument:
                            mIr.playNote(static_cast<trackerboy::Note>(arg));
                            break;
                    }
                    break;
                case Command::previewStop:
                    if (mPreviewState != PreviewState::none) {
                        resetPreview();
                    }
                    break;
                case Command::play: 
                    break;
                case Command::playPattern:
                    break;
                case Command::playFromStart:
                    break;
                case Command::playFromCursor:
                    break;
                case Command::stop:
                    mStopCounter = STOP_FRAMES;
                    // sound off
                    mRc.apu.writeRegister(gbapu::Apu::REG_NR51, 0x00);
                    break;
            }

            if (mStopCounter) {
                // when stop counter is nonzero, we are stopping
                if (--mStopCounter == 0) {
                    mStopped = true;
                    mStopCondition.wakeOne();
                    mDocument.unlock();
                    return;
                }
            } else {

                // step the engine
                trackerboy::Frame frameInfo;
                mEngine.step(frameInfo);

                if (mPreviewState == PreviewState::instrument) {
                    // previewing instrument, step the runtime
                    mIr.step();
                }

                if (frameInfo.halted && mPreviewState == PreviewState::none) {
                    mStopCounter = STOP_FRAMES;
                }

            }

            mFrameRemaining = mSynth.run();
            mFrameBuffer = mSynth.buffer();

            mDocument.unlock();
            
        }

        size_t framesToWrite = std::min(frames, mFrameRemaining);
        size_t samplesToWrite = framesToWrite * 2;
        std::copy_n(mFrameBuffer, samplesToWrite, out);
        mFrameBuffer += samplesToWrite;
        out += samplesToWrite;
        frames -= framesToWrite;
        mFrameRemaining -= framesToWrite;


    }
}

void RendererThread::resetPreview() {
    trackerboy::ChannelControl::writePanning(mPreviewChannel, mRc, 0);
    mEngine.lock(mPreviewChannel);
    mPreviewState = PreviewState::none;
}
