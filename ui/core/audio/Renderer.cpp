
#include "core/audio/Renderer.hpp"
#include "core/samplerates.hpp"

#include "trackerboy/engine/ChannelControl.hpp"

#include <QApplication>
#include <QDeadlineTimer>
#include <QMutexLocker>
#include <QTimerEvent>
#include <QtDebug>

#include <algorithm>
#include <chrono>
#include <type_traits>

static auto LOG_PREFIX = "[Renderer]";


// Renderer Notes
//
// This class is reponsible for rendering audio in real time.The Renderer synthesizes
// audio for playback which is then played out to speakers via the audio callback function.
// Synthesized audio is put into an AudioStream's ringbuffer. This buffer is filled
// completely every period, which is by default 5 ms. The audio callback takes what it
// needs from the buffer. Ideally, the buffer is always at 100% utilization. A lower
// utilization indicates that the callback is consuming faster than the rate the
// audio is being produced. When this happens underruns occur, as the callback doesn't
// get what it needs and there are now gaps in the playback.


Renderer::RenderContext::RenderContext() :
    document(nullptr),
    musicDocument(nullptr),
    stepping(false),
    step(false),
    synth(44100),
    apu(synth.apu()),
    engine(apu, nullptr),
    ip(),
    previewState(PreviewState::none),
    previewChannel(trackerboy::ChType::ch1),
    state(State::stopped),
    stopCounter(0),
    lastPeriod(),
    periodTime(0),
    writesSinceLastPeriod(0)
{
}



Renderer::Renderer(QObject *parent) :
    QObject(parent),
    mMutex(),
    mTimerThread(),
    mTimer(new FastTimer),
    mStream(),
    mVisBuffer(),
    mContext()
{
    mTimer->setCallback(timerCallback, this);
    mTimer->moveToThread(&mTimerThread);
    connect(&mTimerThread, &QThread::finished, mTimer, &FastTimer::deleteLater);
    mTimerThread.setObjectName(QStringLiteral("renderer timer thread"));
    mTimerThread.start();
}

Renderer::~Renderer() {
    mTimer->stop();

    if (mStream.isRunning()) {
        mStream.stop();
    }

    mTimerThread.quit();
    mTimerThread.wait();
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

ModuleDocument* Renderer::document() {
    auto handle = mContext.access();
    return handle->document;
}

ModuleDocument* Renderer::documentPlayingMusic() {
    auto handle = mContext.access();
    return handle->musicDocument;
}

trackerboy::Frame Renderer::currentFrame() {
    auto handle = mContext.access();
    return handle->currentEngineFrame;
}

bool Renderer::setConfig(Config::Sound const &soundConfig) {

    // if there is rendering going at on when this function is called it will
    // resume with a slight gap in playback if the config applied without error,
    // otherwise the render is stopped


    bool wasRunning = mStream.isRunning();
    if (wasRunning) {
        mTimer->stop();
    }

    mStream.setConfig(soundConfig);

    if (mStream.isEnabled()) {

        mTimer->setInterval(soundConfig.period, Qt::PreciseTimer);
        

        // update the synthesizer (the guard isn't necessary here but we'll use it anyways)
        {
            auto handle = mContext.access();
            
            bool reloadRegisters = false;
            auto const samplerate = SAMPLERATE_TABLE[soundConfig.samplerateIndex];
            if (samplerate != handle->synth.samplerate()) {
                handle->synth.setSamplingRate(samplerate);
                reloadRegisters = wasRunning;
            }
            handle->synth.apu().setQuality(static_cast<gbapu::Apu::Quality>(soundConfig.quality));
            handle->synth.setupBuffers();

            if (reloadRegisters) {
                // resizing the buffers in synth results in an APU reset so we need to
                // rewrite channel registers
                handle->engine.reload();
            }


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
    
    mTimer->stop();

    auto success = mStream.stop();

    handle.unlock();

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
}



// SLOTS

void Renderer::clearDiagnostics() {
    mStream.resetUnderruns();
}

void Renderer::setDocument(ModuleDocument *doc) {

    mContext.access()->document = doc;

    stopPreview();
}

void Renderer::play() {

    if (mStream.isEnabled()) {
        auto handle = mContext.access();
        if (handle->stepping) {
            // stop step mode
            handle->stepping = false;
        } else {
            playMusic(handle, handle->document->orderModel().currentPattern(), 0);
        }
    }
}

void Renderer::playAtStart() {
    
    if (mStream.isEnabled()) {
        auto handle = mContext.access();
        playMusic(handle, 0, 0);
    }
}

void Renderer::playFromCursor() {
    
    if (mStream.isEnabled()) {
        auto handle = mContext.access();
        playMusic(
            handle,
            handle->document->orderModel().currentPattern(),
            handle->document->patternModel().cursorRow()
        );
    }
}

void Renderer::stepFromCursor() {
    
    if (mStream.isEnabled()) {
        auto handle = mContext.access();
        if (!handle->stepping) {
            playMusic(
                handle,
                handle->document->orderModel().currentPattern(), 
                handle->document->patternModel().cursorRow(), 
                true
            );
        }

        handle->step = true;

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

void Renderer::previewNoteOrInstrument(int note, int track, int instrument) {
    
    if (mStream.isEnabled()) {
        auto handle = mContext.access();
        switch (handle->previewState) {
            case PreviewState::waveform:
                resetPreview(handle);
                [[fallthrough]];
            case PreviewState::none: {

                std::shared_ptr<trackerboy::Instrument> inst = nullptr;
                if (track == -1) {
                    // instrument preview
                    // set instrument preview's instrument to the current one
                    inst = handle->document->instrumentModel().currentInstrument();
                    handle->previewChannel = inst->channel();
                } else {
                    // note preview
                    if (instrument != -1) {
                        inst = handle->document->mod().instrumentTable().getShared((uint8_t)instrument);
                    }
                    handle->previewChannel = static_cast<trackerboy::ChType>(track);
                }
                handle->ip.setInstrument(std::move(inst), handle->previewChannel);

                handle->previewState = PreviewState::instrument;
                // unlock the channel for preview
                handle->engine.unlock(handle->previewChannel);
            }
                [[fallthrough]];
            case PreviewState::instrument:
                // update the current note
                handle->ip.play((uint8_t)note);
                break;
        }

        beginRender(handle);
    }
}

void Renderer::previewInstrument(quint8 note) {
    previewNoteOrInstrument(note);
}

void Renderer::previewNote(int note, int track, int instrument) {
    previewNoteOrInstrument(note, track, instrument);
}

void Renderer::previewWaveform(quint8 note) {
    if (mStream.isEnabled()) {
        // state changes
        // instrument -> none -> waveform

        if (note > trackerboy::NOTE_LAST) {
            // should never happen, but clamp just in case
            note = trackerboy::NOTE_LAST;
        }
        auto freq = trackerboy::NOTE_FREQ_TABLE[note];

        auto handle = mContext.access();
        switch (handle->previewState) {
            case PreviewState::instrument:
                resetPreview(handle);
                [[fallthrough]];
            case PreviewState::none: {
                handle->previewState = PreviewState::waveform;
                handle->previewChannel = trackerboy::ChType::ch3;
                // unlock the channel, no longer effected by music
                handle->engine.unlock(trackerboy::ChType::ch3);

                trackerboy::ChannelState state(trackerboy::ChType::ch3);
                state.playing = true;
                state.frequency = freq;
                state.envelope = handle->document->waveModel().currentWaveform()->id();
                trackerboy::ChannelControl<trackerboy::ChType::ch3>::init(handle->apu, handle->document->mod().waveformTable(), state);
                break;
            }
            case PreviewState::waveform:
                // already previewing, just update the frequency
                handle->apu.writeRegister(gbapu::Apu::REG_NR33, (uint8_t)(freq & 0xFF));
                handle->apu.writeRegister(gbapu::Apu::REG_NR34, (uint8_t)(freq >> 8));
                break;
        }

        beginRender(handle);
    }
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
        handle->engine.halt();
        handle->stepping = false;
    }

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

void Renderer::playMusic(Handle &handle, int orderNo, int rowNo, bool stepping) {

    if (handle->musicDocument != handle->document) {
        if (handle->musicDocument) {
            handle->musicDocument->disconnect(this);
        }

        handle->musicDocument = handle->document;

        handle->engine.setModule(&handle->document->mod());
        connect(handle->musicDocument, &ModuleDocument::channelOutputChanged, this, &Renderer::setChannelOutput);
    }
    handle->engine.play(orderNo, rowNo);
    // unlock disabled channels
    _setChannelOutput(handle, handle->musicDocument->channelOutput());

    handle->stepping = stepping;
    
    beginRender(handle);

}

void Renderer::resetPreview(Handle &handle) {
    // lock the channel so it can be used for music
    handle->engine.lock(handle->previewChannel);
    handle->ip.setInstrument(nullptr);
    handle->previewState = PreviewState::none;
}

void Renderer::removeDocument(ModuleDocument *doc) {
    auto handle = mContext.access();
    if (handle->musicDocument == doc) {
        if (!handle->currentEngineFrame.halted && handle->state != State::stopped) {
            // we are currently renderering this document, stop immediately
            // since doc will no longer exist after this function
            stopRender(handle);
        }
        handle->engine.setModule(nullptr);
        handle->musicDocument = nullptr;

    }
    
}

void Renderer::setChannelOutput(ModuleDocument::OutputFlags flags) {
    auto handle = mContext.access();
    _setChannelOutput(handle, flags);
}

void Renderer::_setChannelOutput(Handle &handle, ModuleDocument::OutputFlags flags) {
    int flag = ModuleDocument::CH1;
    for (int i = 0; i < 4; ++i) {
        auto ch = static_cast<trackerboy::ChType>(i);
        if (flags.testFlag((ModuleDocument::OutputFlag)(flag))) {
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

    // make sure the stream is still running
    if (!mStream.isRunning()) {
        // stream isn't running and it should be, abort the render
        // this typically means a device has been disconnected and can no longer be used
        stopRender(handle, true);
    }

    auto frame = handle->currentEngineFrame;

    auto writer = mStream.writer();
    auto framesToRender = writer.availableWrite();

    // diagnostics
    handle->periodTime = now - handle->lastPeriod;
    handle->lastPeriod = now;
    handle->writesSinceLastPeriod = 0;

    // cache a ref to the apu, we'll be using it often
    auto &apu = handle->synth.apu();

    bool newFrame = false;

    auto visHandle = mVisBuffer.access();
    visHandle->beginWrite(framesToRender);

    while (framesToRender) {

        if (handle->state == State::stopping) {
            if (writer.availableWrite() == mStream.bufferSize()) {
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
                    // so their corresponding documents must be locked when
                    // stepping

                    // step engine/previewer
                    bool const lockMusicDocument = handle->musicDocument != nullptr;
                    if (lockMusicDocument) {
                        handle->musicDocument->lock();
                    }

                    if (!handle->stepping || handle->step) {
                        handle->engine.step(frame);
                        if (frame.startedNewRow) {
                            handle->step = false;
                        }
                    }

                    if (lockMusicDocument) {
                        handle->musicDocument->unlock();
                    }

                    if (handle->previewState == PreviewState::instrument) {

                        handle->document->lock();
                        auto &mod = handle->document->mod();
                        trackerboy::RuntimeContext rc(handle->apu, mod.instrumentTable(), mod.waveformTable());
                        handle->ip.step(rc);
                        handle->document->unlock();
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
        emit frameSync();
    }

}
