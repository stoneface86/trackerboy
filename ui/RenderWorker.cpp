
#include "RenderWorker.hpp"

#include <QMutexLocker>

RenderWorker::RenderWorker(ModuleDocument &document) :
    mDocument(document),
    mPb(audio::SR_44100),
    mSynth(44100.0f),
    mEngine({mSynth, document.instrumentTable(), document.waveTable()}),
    mPreviewState(PreviewState::none),
    mPreviewChannel(trackerboy::ChType::ch1),
    mRendering(false),
    mMusicPlaying(false),
    QObject()
{
}

void RenderWorker::previewWaveform(trackerboy::Note note) {
    QMutexLocker locker(&mMutex);

    auto &gen3 = mSynth.hardware().gen3;
    if (mPreviewState == PreviewState::none) {
        gen3.copyWave(*(mDocument.waveListModel()->currentWaveform()));
        mSynth.setOutputEnable(trackerboy::ChType::ch3, trackerboy::Gbs::TERM_BOTH, true);
        mPreviewState = PreviewState::waveform;
        mPreviewChannel = trackerboy::ChType::ch3;
        mEngine.unlock(trackerboy::ChType::ch3);
    }

    gen3.setFrequency(static_cast<int>(trackerboy::NOTE_FREQ_TABLE[note]));

}

void RenderWorker::stopPreview() {
    QMutexLocker locker(&mMutex);
    if (mPreviewState != PreviewState::none) {
        mEngine.lock(mPreviewChannel);
        mPreviewState = PreviewState::none;
    }
}

bool RenderWorker::isRendering() {
    QMutexLocker locker(&mMutex);
    return mRendering;
}

void RenderWorker::quitRender() {
    QMutexLocker locker(&mMutex);
    mMusicPlaying = false;
    mPreviewState = PreviewState::none;
}


void RenderWorker::render() {

    mMutex.lock();
    mRendering = true;
    mMutex.unlock();

    bool wait = false;

    mPb.silence();
    mPb.start();

    for (;;) {
        mMutex.lock();
        if (mPreviewState == PreviewState::none && mMusicPlaying == false) {
            mMutex.unlock();
            break;
        }

        bool halted = mEngine.step();
        // if the engine halted and we are not previewing, we will stop rendering after this frame
        bool stopAfterFrame = halted && mPreviewState == PreviewState::none;

        // synthesize the frame
        size_t framesize = mSynth.run();
        mMutex.unlock();

        // write the frame to the queue, will block if the queue is full
        mPb.writeAll(mSynth.buffer(), framesize);

        if (stopAfterFrame) {
            
            wait = true;
            mMutex.lock();
            mMusicPlaying = false;
            mMutex.unlock();
            break;
        }

    }


    mPb.stop(wait);

    mMutex.lock();
    mRendering = false;
    mMutex.unlock();
}
