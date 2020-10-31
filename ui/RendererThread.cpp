
#include "RendererThread.hpp"

#include <cstdint>

// TODO: the callback thread should buffer more than 1 frame. Gaps may occur if
// locking the spinlock fails when rendering the frame. More frames gives us
// more chances at the cost of latency.



RendererThread::RendererThread(ModuleDocument &document,
                               InstrumentListModel &instrumentModel,
                               WaveListModel &waveModel,
                               QWaitCondition &stopCondition
) :
    mStopCondition(stopCondition),
    mRunning(false),
    mDocument(document),
    mInstrumentModel(instrumentModel),
    mWaveModel(waveModel),
    mSynth(44100),
    mRc(mSynth, document.instrumentTable(), document.waveTable()),
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
    config.playback.format = ma_format_s16;
    config.playback.channels = 2;
    config.sampleRate = soundConfig.samplerate;
    config.dataCallback = audioCallback;
    config.pUserData = this;

    mDevice.emplace();
    auto err = ma_device_init(nullptr, &config, &mDevice.value());
    assert(err == MA_SUCCESS);

    mSynth.setSamplingRate(soundConfig.samplerate);
    mSynth.setVolume(soundConfig.volume);
    mSynth.setupBuffers();
}

void RendererThread::close() {
    if (mDevice) {
        ma_device_uninit(&mDevice.value());
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
                            mSynth.setOutputEnable(trackerboy::ChType::ch3, trackerboy::Gbs::TERM_BOTH, true);
                            mSynth.setWaveram(*(mWaveModel.currentWaveform()));

                            [[fallthrough]];
                        case PreviewState::waveform:
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
                    mSynth.writeRegister(trackerboy::Gbs::REG_NR51, 0x00);
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
    mSynth.setOutputEnable(mPreviewChannel, trackerboy::Gbs::TERM_BOTH, false);
    mEngine.lock(mPreviewChannel);
    mPreviewState = PreviewState::none;
}
