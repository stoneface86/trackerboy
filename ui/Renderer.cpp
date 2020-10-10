
#include "Renderer.hpp"

#include <QTime>

#include <chrono>


Renderer::Renderer(ModuleDocument &document, InstrumentListModel &instrumentModel, WaveListModel &waveModel, QObject *parent) :
    mDocument(document),
    mInstrumentModel(instrumentModel),
    mWaveModel(waveModel),
    mPb(audio::SR_44100),
    mSynth(44100),
    mEngine({ mSynth, document.instrumentTable(), document.waveTable() }),
    mIr({ mSynth, document.instrumentTable(), document.waveTable() }),
    mPreviewState(PreviewState::none),
    mPreviewChannel(trackerboy::ChType::ch1),
    //mRendering(false),
    mMusicPlaying(false),
    mRunning(true),
    mIdling(true),
    QThread(parent)
{
    mPb.open();
}

Renderer::~Renderer() {

    mMutex.lock();
    mRunning = false;
    mMutex.unlock();

    stopPreview();
    stopMusic();
    stopIdling();

    wait();

    mPb.close();
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

    auto &gen3 = mSynth.hardware().gen3;
    switch (mPreviewState) {
        case PreviewState::instrument:
            resetPreview();
            [[fallthrough]];
        case PreviewState::none:
            gen3.copyWave(*(mWaveModel.currentWaveform()));
            mSynth.setOutputEnable(trackerboy::ChType::ch3, trackerboy::Gbs::TERM_BOTH, true);
            mPreviewState = PreviewState::waveform;
            mPreviewChannel = trackerboy::ChType::ch3;
            mEngine.unlock(trackerboy::ChType::ch3);

            stopIdling();
            [[fallthrough]];
        case PreviewState::waveform:
            gen3.setFrequency(static_cast<int>(trackerboy::NOTE_FREQ_TABLE[note]));
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


void Renderer::run() {
    bool running = true;

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
        running = mRunning;
        mMutex.unlock();
        if (!running) {
            // no longer running, the thread will stop executing
            break;
        }
        
        for (;;) {

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

            // write the frame to the queue, will block if the queue is full
            try {
                mPb.writeAll(mSynth.buffer(), framesize);
            } catch (audio::PaException paexcept) {
                // if we get here then we could not open a stream for the configured device
                // just stop everything

                // TODO: have some kind of error reporting mechanism here (maybe an error signal)
                mMusicPlaying = false;
                mPreviewState = PreviewState::none;
                break;
            }

            if (stopAfterFrame) {

                mMutex.lock();
                mMusicPlaying = false;
                mMutex.unlock();
                break;
            }
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



