
#include "Renderer.hpp"

#include <QTime>

#include <chrono>
#include <thread>


Renderer::Renderer(ModuleDocument &document, InstrumentListModel &instrumentModel, WaveListModel &waveModel, Config &config, QObject *parent) :
    QThread(parent),
    mDocument(document),
    mInstrumentModel(instrumentModel),
    mWaveModel(waveModel),
    mConfig(config),
    mPb(),
    mSynth(44100),
    mRc(mSynth, document.instrumentTable(), document.waveTable()),
    mEngine(mRc),
    mIr(mRc),
    mPreviewState(PreviewState::none),
    mPreviewChannel(trackerboy::ChType::ch1),
    //mRendering(false),
    mMusicPlaying(false),
    mRunning(true),
    mIdling(true)
{
    connect(&mConfig, &Config::soundConfigChanged, this, &Renderer::onSoundChange);
    //mPb.open();
}

Renderer::~Renderer() {
    mMutex.lock();
    mRunning = false;
    mMutex.unlock();

    stopPreview();
    stopMusic();
    stopIdling();

    wait();

    //mPb.close();
}

void Renderer::play() {

}

void Renderer::playPattern() {

}

void Renderer::playFromStart() {

}

void Renderer::playFromCursor() {

}

void Renderer::previewWaveform(trackerboy::Note note) {
    QMutexLocker locker(&mMutex);

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

            stopIdling();
            [[fallthrough]];
        case PreviewState::waveform:
            trackerboy::ChannelControl::writeFrequency(
                        trackerboy::ChType::ch3,
                        mRc,
                        trackerboy::NOTE_FREQ_TABLE[note]
                        );
            break;
    }
    

    
}

void Renderer::previewInstrument(trackerboy::Note note) {
    QMutexLocker locker(&mMutex);

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
            mIr.playNote(note);

            stopIdling();
            [[fallthrough]];
        case PreviewState::instrument:
            mIr.playNote(note);
            break;
    }

    
}

void Renderer::stopPreview() {
    QMutexLocker locker(&mMutex);
    if (mPreviewState != PreviewState::none) {
        resetPreview();
    }
}

void Renderer::stopMusic() {

}

void Renderer::stop() {

}


void Renderer::run() {
    mMutex.lock();
    mRunning = true;
    mMutex.unlock();

    for (;;) {

        // wait until we need to quit or begin rendering again
        mIdlingMutex.lock();
        if (hasNoWork()) {
            mIdling = true;
            mIdlingCond.wait(&mIdlingMutex);
            mIdling = false;
        }
        mIdlingMutex.unlock();

        // check if we are still running
        mMutex.lock();
        bool running = mRunning;
        mMutex.unlock();
        if (!running) {
            // no longer running, the thread will stop executing
            break;
        }
        
        //auto nextFrame = std::chrono::steady_clock::now();
        for (;;) {

            //nextFrame += std::chrono::microseconds((int)(1000000 / 59.7));

            if (hasNoWork()) {
                break;
            }

            mMutex.lock();
            mDocument.lock(); // lock document so that changes do not occur during engine step
            if (mPreviewState == PreviewState::instrument) {
                mIr.step();
            }

            trackerboy::Frame frame;
            mEngine.step(frame);
            mDocument.unlock();

            // if the engine halted and we are not previewing, we will stop rendering after this frame
            bool stopAfterFrame = frame.halted && mPreviewState == PreviewState::none;

            // synthesize the frame
            size_t framesize = mSynth.run();
            mMutex.unlock();

            mPb.enqueue(mSynth.buffer(), framesize);

            // write the frame to the queue, will block if the queue is full
            //try {
            //    mPb.writeAll(mSynth.buffer(), framesize);
            //} catch (audio::SoundIoError err) {
            //    // if we get here then we could not open a stream for the configured device
            //    // just stop everything

            //    // TODO: have some kind of error reporting mechanism here (maybe an error signal)
            //    mMusicPlaying = false;
            //    mPreviewState = PreviewState::none;
            //    break;
            //}

            if (stopAfterFrame) {

                mMutex.lock();
                mMusicPlaying = false;
                mMutex.unlock();
                break;
            }

            //std::this_thread::sleep_until(nextFrame);
        }

        mPb.stop(true);
    }
}

void Renderer::resetPreview() {
    mSynth.setOutputEnable(mPreviewChannel, trackerboy::Gbs::TERM_BOTH, false);
    mEngine.lock(mPreviewChannel);
    mPreviewState = PreviewState::none;
}

void Renderer::stopIdling() {
    mIdlingMutex.lock();
    bool idling = mIdling;
    mIdlingMutex.unlock();


    if (idling) {
        // wake the (one and only) rendering thread
        mIdlingCond.wakeOne();
    }
}

bool Renderer::hasNoWork() {
    QMutexLocker locker(&mMutex);
    return mPreviewState == PreviewState::none && !mMusicPlaying;
}


void Renderer::onSoundChange() {

    if (isRunning()) {
        // stop the thread
        mMutex.lock();
        mRunning = false;
        mMutex.unlock();

        stopIdling();

        wait();
    }

    auto &sound = mConfig.sound();

    // whatever is queued will be lost
    //mPb.stop(false);
    /*mPb.close();

    mPb.setDevice(sound.device, sound.samplerate);
    mPb.open();

    mSynth.setSamplingRate(audio::SAMPLERATE_TABLE[sound.samplerate]);*/
    //mSynth.setBass(sound.bassFrequency);
    //mSynth.setTreble(sound.treble, sound.trebleFrequency);

    //double gain = pow(2.0, sound.volume / 6.0);
    //mSynth.setVolume(gain);

    mSynth.setupBuffers();

    mPb.open();
    //mPb.setLatency(50);
    start(QThread::Priority::TimeCriticalPriority);
}


