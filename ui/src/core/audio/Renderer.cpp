
#include "core/audio/Renderer.hpp"
#include "core/samplerates.hpp"
#include "core/misc/utils.hpp"

#include "trackerboy/engine/ChannelControl.hpp"

#include <QMutexLocker>
#include <QtDebug>

//static auto LOG_PREFIX = "[Renderer]";


// Renderer Notes
//
// This class is reponsible for rendering audio in real time. The Renderer synthesizes
// audio for playback which is then played out to speakers via the audio callback function.
// Synthesized audio is put into an AudioStream's ringbuffer. This buffer is filled
// completely every period, which is by default 5 ms. The audio callback takes what it
// needs from the buffer. Ideally, the buffer is always at 100% utilization. A lower
// utilization indicates that the callback is consuming faster than the rate the
// audio is being produced. When this happens underruns occur, as the callback doesn't
// get what it needs and there are now gaps in the playback.


Renderer::RenderContext::RenderContext(Module &mod) :
    mod(mod),
    stepping(false),
    step(false),
    song(nullptr),
    synth(44100),
    apu(synth.apu()),
    engine(apu, &mod.data()),
    ip(),
    previewState(PreviewState::none),
    previewChannel(trackerboy::ChType::ch1),
    state(State::stopped),
    stopCounter(0),
    bufferSize(0),
    watchdog(),
    lastPeriod(),
    periodTime(0),
    writesSinceLastPeriod(0)
{
}



Renderer::Renderer(Module &mod, QObject *parent) :
    QObject(parent),
    mTimerThread(),
    mTimer(new FastTimer),
    mStream(),
    mVisBuffer(),
    mOutputFlags(ChannelOutput::AllOn),
    mContext(mod)
{
    mTimer->setCallback(timerCallback, this);
    mTimer->moveToThread(&mTimerThread);
    connect(&mTimerThread, &QThread::finished, mTimer, &FastTimer::deleteLater);
    mTimerThread.setObjectName(QStringLiteral("renderer timer thread"));
    mTimerThread.start();

    connect(&mStream, &AudioStream::aborted, this,
        [this]() {
            auto handle = mContext.access();
            stopRender(handle, true);
        });

    connect(&mod, &Module::songChanged, this, &Renderer::setSong);
    setSong();
}

Renderer::~Renderer() {
    mTimer->stop();

    if (mStream.isRunning()) {
        mStream.stop();
    }

    mTimerThread.quit();
    mTimerThread.wait();
}

void Renderer::setSong() {
    auto ctx = mContext.access();
    ctx->song = ctx->mod.songShared();
    ctx->engine.setSong(ctx->song.get());

    // if we are playing, restart playback from the start with the new song
    // if we are stepping, stop playback

    if (mStream.isRunning()) {
        if (ctx->stepping) {
            _stopMusic(ctx);
        } else {
            _play(ctx, 0, 0, false);
        }
    }
}

Renderer::Diagnostics Renderer::diagnostics() {
    auto handle = mContext.access();

    auto size = mStream.bufferSize();
    // it is safe to access the writer since we have acquired access to context
    auto usage = size - mStream.writer().availableWrite();


    return {
        mStream.underruns(),
        usage,
        size,
        handle->writesSinceLastPeriod,
        handle->periodTime,
        mStream.elapsed()
    };
}

Guarded<VisualizerBuffer>& Renderer::visualizerBuffer() {
    return mVisBuffer;
}

bool Renderer::isRunning() {
    return mStream.isRunning();
}

bool Renderer::isStepping() {
    return mContext.access()->stepping;
}

bool Renderer::isPlaying() {
    return !mContext.access()->currentEngineFrame.halted;
}

trackerboy::Frame Renderer::currentFrame() {
    auto handle = mContext.access();
    return handle->currentEngineFrame;
}

bool Renderer::setConfig(SoundConfig const &soundConfig) {

    // if there is rendering going at on when this function is called it will
    // resume with a slight gap in playback if the config applied without error,
    // otherwise the render is stopped


    bool wasRunning = mStream.isRunning();
    if (wasRunning) {
        mTimer->stop();
    }

    mStream.setConfig(soundConfig);

    if (mStream.isEnabled()) {

        mTimer->setInterval(soundConfig.period(), Qt::PreciseTimer);
        

        // update the synthesizer (the guard isn't necessary here but we'll use it anyways)
        {
            auto handle = mContext.access();
            
            bool reloadRegisters = false;
            auto const samplerate = soundConfig.samplerate();
            if (samplerate != handle->synth.samplerate()) {
                handle->synth.setSamplerate(samplerate);
                reloadRegisters = wasRunning;
            }
            handle->synth.apu().setQuality(static_cast<gbapu::Apu::Quality>(soundConfig.quality()));
            handle->synth.setupBuffers();

            if (reloadRegisters) {
                // resizing the buffers in synth results in an APU reset so we need to
                // rewrite channel registers
                handle->engine.reload();
            }

            handle->bufferSize = mStream.bufferSize();


            mVisBuffer.access()->resize(handle->synth.framesize());



        }

        if (wasRunning && mStream.isRunning()) {
            mTimer->start();
        }

        return true;

    } else {
        // something went wrong
        mContext.access()->state = State::stopped;
        return false;
    }
}

void Renderer::beginRender(Handle &handle) {
    if (handle->state == State::stopped) {

        bool success = mStream.start();

        if (success) {
            handle->lastPeriod = Clock::now();
            handle->watchdog = handle->lastPeriod;
            mTimer->start();
            handle.unlock();
            emit audioStarted();
            handle.relock();
        } else {
            // unable to start, an error occurred
            handle.unlock();
            emit audioError();
            return;
        }

    }

    // reset state to running
    handle->state = State::running;
    handle->stopCounter = 0;
}

void Renderer::stopRender(Handle &handle, bool aborted) {

    handle->state = State::stopped;
    handle.unlock();

    // this lambda must only be called from the same thread as the Renderer
    auto stopRender_ = [this, aborted]() {
        mTimer->stop();

        auto success = mStream.stop();

        mVisBuffer.access()->clear();
        emit updateVisualizers();

        if (aborted) {
            mStream.disable();
            emit audioError();
        } else {
            if (success) {
                emit audioStopped();
            } else {
                emit audioError();
            }
        }
    };

    // determine if we are in the GUI thread (same thread as the Renderer)
    // this function is mostly called from the timer thread, occurs when:
    //  - the buffer has drained and we are stopping
    //  - the watchdog timer has exceeded 1 second (unknown problem with device)
    // for these cases we need to invoke the lambda in the GUI thread (AudioStream is not thread-safe)

    if (objectInCurrentThread(*this)) {
        stopRender_();
    } else {
        // stopRender was called from mTimerThread, invoke the lambda in the renderer's thread
        QMetaObject::invokeMethod(this, stopRender_);
    }


}



// SLOTS

void Renderer::clearDiagnostics() {
    mStream.resetUnderruns();
}

void Renderer::play(int pattern, int row, bool stepmode) {

    if (mStream.isEnabled()) {
        auto handle = mContext.access();
        _play(handle, pattern, row, stepmode);
    }
}


void Renderer::stepNextFrame() {
    
    if (mStream.isEnabled()) {
        auto handle = mContext.access();
        if (handle->stepping) {
            handle->step = true;
        }

    }
}

void Renderer::stepOut() {
    if (mStream.isEnabled()) {
        mContext.access()->stepping = false;
    }
}

void Renderer::jumpToPattern(int pattern) {
    if (mStream.isEnabled()) {
        auto ctx = mContext.access();
        ctx->engine.jump(pattern);
    }
}

void Renderer::setPatternRepeat(bool repeat) {

    if (mStream.isEnabled()) {
        mContext.access()->engine.repeatPattern(repeat);
    }
}

void Renderer::setPreviewNote(int note) {
    if (mStream.isEnabled()) {
        auto ctx = mContext.access();
        switch (ctx->previewState) {
            case PreviewState::waveform: {
                if (note > trackerboy::NOTE_LAST) {
                    // should never happen, but clamp just in case
                    note = trackerboy::NOTE_LAST;
                }
                auto freq = trackerboy::NOTE_FREQ_TABLE[note];
                ctx->apu.writeRegister(gbapu::Apu::REG_NR33, (uint8_t)(freq & 0xFF));
                ctx->apu.writeRegister(gbapu::Apu::REG_NR34, (uint8_t)(freq >> 8));
                break;
            }
            case PreviewState::instrument:
                // update the current note
                ctx->ip.play((uint8_t)note);
                break;
            default:
                break;
        
        }
    }
}

void Renderer::instrumentPreview(int note, int track, int instrumentId) {
    if (mStream.isEnabled()) {
        auto ctx = mContext.access();
        switch (ctx->previewState) {
            case PreviewState::instrument:
            case PreviewState::waveform:
                resetPreview(ctx);
                [[fallthrough]];
            case PreviewState::none: {
                auto const& itable = ctx->mod.data().instrumentTable();
                std::shared_ptr<trackerboy::Instrument> inst = nullptr;
                if (instrumentId != -1) {
                    inst = itable.getShared((uint8_t)instrumentId);
                }

                if (track == -1) {
                    // instrument preview
                    Q_ASSERT(inst != nullptr); // must have an instrument
                    ctx->previewChannel = inst->channel();
                } else {
                    // note preview
                    ctx->previewChannel = static_cast<trackerboy::ChType>(track);
                }

                ctx->ip.setInstrument(std::move(inst), ctx->previewChannel);

                ctx->previewState = PreviewState::instrument;
                // unlock the channel for preview
                ctx->engine.unlock(ctx->previewChannel);
                ctx->ip.play((uint8_t)note);
                break;
            }
        }
        beginRender(ctx);
    }
}

void Renderer::waveformPreview(int note, int waveId) {
    if (mStream.isEnabled()) {
        auto ctx = mContext.access();
        switch (ctx->previewState) {
            case PreviewState::instrument:
            case PreviewState::waveform:
                resetPreview(ctx);
                [[fallthrough]];
            case PreviewState::none:
                ctx->previewState = PreviewState::waveform;
                ctx->previewChannel = trackerboy::ChType::ch3;
                // unlock the channel, no longer effected by music
                ctx->engine.unlock(trackerboy::ChType::ch3);

                trackerboy::ChannelState state(trackerboy::ChType::ch3);
                state.playing = true;
                state.frequency = trackerboy::NOTE_FREQ_TABLE[note];
                state.envelope = (uint8_t)waveId;
                trackerboy::ChannelControl<trackerboy::ChType::ch3>::init(
                    ctx->apu, ctx->mod.data().waveformTable(), state
                );
                break;
        }
        beginRender(ctx);
    }
}

void Renderer::updateFramerate() {
    auto ctx = mContext.access();
    ctx->synth.setFramerate(ctx->mod.data().framerate());
    ctx->synth.setupBuffers();
}

void Renderer::stopPreview() {

    if (mStream.isEnabled()) {
        auto handle = mContext.access();
        if (handle->previewState != PreviewState::none) {
            resetPreview(handle);
        }
    }
    
}

void Renderer::stopMusic() {
    
    if (mStream.isEnabled()) {
        auto handle = mContext.access();
        _stopMusic(handle);
    }

}

void Renderer::_stopMusic(Handle &handle) {
    handle->engine.halt();
    handle->stepping = false;
}

void Renderer::forceStop() {

    if (mStream.isEnabled()) {
        auto handle = mContext.access();
        if (handle->state != State::stopped) {
            resetPreview(handle);
            handle->engine.halt();
            handle->stepping = false;
            stopRender(handle);
        }
    }
}

void Renderer::_play(Handle &handle, int orderNo, int rowNo, bool stepping) {

    handle->engine.play(orderNo, rowNo);
    _setChannelOutput(handle, mOutputFlags);
    handle->stepping = stepping;
    handle->step = stepping;
    beginRender(handle);

}

void Renderer::resetPreview(Handle &handle) {
    // lock the channel so it can be used for music
    handle->engine.lock(handle->previewChannel);
    handle->ip.setInstrument(nullptr);
    handle->previewState = PreviewState::none;
}

 void Renderer::setChannelOutput(ChannelOutput::Flags flags) {
     mOutputFlags = flags;
     auto ctx = mContext.access();
     _setChannelOutput(ctx, flags);
 }

 void Renderer::_setChannelOutput(Handle &handle, ChannelOutput::Flags flags) {
     int flag = ChannelOutput::CH1;
     for (int i = 0; i < 4; ++i) {
         auto ch = static_cast<trackerboy::ChType>(i);
         if (flags.testFlag((ChannelOutput::Flag)(flag))) {
             handle->engine.lock(ch);
         } else {
             // channel is disabled, keep unlocked
             handle->engine.unlock(ch);
         }
         flag <<= 1;
     }
 }

void Renderer::timerCallback(void *userData) {
    // called by FastTimer 
    static_cast<Renderer*>(userData)->render();
}

// this is the number of frames to output before stopping playback
// (prevents a hard pop noise that may occur when stopping abruptly, as
// the high pass filter will decay the signal to 0)
constexpr int STOP_FRAMES = 5;

void Renderer::render() {
    // This function is called from a separate thread!
    // FastTimer lives in its own thread and calls this function via the timer callback
    
    auto now = Clock::now();

    auto handle = mContext.access();

    if (handle->state == State::stopped) {
        return;
    }


    // diagnostics
    handle->periodTime = now - handle->lastPeriod;
    handle->lastPeriod = now;
    handle->writesSinceLastPeriod = 0;


    auto writer = mStream.writer();
    auto framesToRender = writer.availableWrite();

    if (framesToRender) {
        // reset the watchdog
        handle->watchdog = now;
    } else {
        constexpr auto WATCHDOG_INTERVAL = std::chrono::seconds(1);
        auto timeSinceLastWatchdogReset = now - handle->watchdog;
        if (timeSinceLastWatchdogReset >= WATCHDOG_INTERVAL) {
            // we have gone 1 second without renderering anything
            // abort the render
            stopRender(handle, true);
        }
        // no frames to render, exit early
        return;
    }

    
    auto frame = handle->currentEngineFrame;
    auto const haltedBefore = frame.halted;

    // cache a ref to the apu, we'll be using it often
    auto &apu = handle->synth.apu();

    bool newFrame = false;

    auto visHandle = mVisBuffer.access();
    visHandle->beginWrite(framesToRender);

    while (framesToRender) {

        if (handle->state == State::stopping) {
            if (writer.availableWrite() == handle->bufferSize) {
                // the buffer has been drained, stop the callback
                visHandle.unlock();
                stopRender(handle);
            }
            return;

        } else {

            if (apu.availableSamples() == 0) {
                // new frame

                if (handle->state == State::stopping) {
                    break; // stop, don't render any more
                }

                if (handle->stopCounter) {
                    if (--handle->stopCounter == 0) {
                        handle->state = State::stopping;
                    }
                } else {
                    newFrame = true;

                    // the engine and previewer have read access to the module
                    // so the document must be locked when stepping

                    // step engine/previewer
                    if (!handle->stepping || handle->step) {
                        
                        {
                            QMutexLocker locker(&handle->mod.mutex());
                            handle->engine.step(frame);
                        }
                        
                        if (frame.startedNewRow) {
                            handle->step = false;
                        }
                    }

                    if (handle->previewState == PreviewState::instrument) {
                        auto &mod = handle->mod.data();
                        trackerboy::RuntimeContext rc(handle->apu, mod.instrumentTable(), mod.waveformTable());
                        
                        {
                            QMutexLocker locker(&handle->mod.mutex());
                            handle->ip.step(rc);
                        }
                    }


                    if (frame.halted && handle->previewState == PreviewState::none) {
                        // no longer doing anything, start the stop counter
                        handle->stopCounter = STOP_FRAMES;
                    }

                }

                handle->synth.run();

            }

            size_t toWrite = std::min(framesToRender, apu.availableSamples());
            auto writePtr = writer.acquireWrite(toWrite);
            
            // read from the apu to the ringbuffer
            apu.readSamples(writePtr, toWrite);
            // send a copy to the visualizer buffer as well
            visHandle->write(writePtr, toWrite);
            
            writer.commitWrite(writePtr, toWrite);
            
            handle->writesSinceLastPeriod += toWrite;
            framesToRender -= toWrite;

        }

    }

    if (handle->writesSinceLastPeriod) {
        emit updateVisualizers();
    }

    if (newFrame) {
        handle->currentEngineFrame = frame;
        handle.unlock(); // always unlock before emitting signals
        if (haltedBefore != frame.halted) {
            emit isPlayingChanged(!frame.halted);
        }
        emit frameSync();
    }

}
