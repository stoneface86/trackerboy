
#include "RenderWorker.hpp"

#include <QMutexLocker>

RenderWorker::RenderWorker(ModuleDocument &document, InstrumentListModel &instrumentModel, WaveListModel &waveModel) :
    mDocument(document),
    mInstrumentModel(instrumentModel),
    mWaveModel(waveModel),
    mPb(audio::SR_44100),
    mSynth(44100.0f),
    mEngine({mSynth, document.instrumentTable(), document.waveTable()}),
    mIr({ mSynth, document.instrumentTable(), document.waveTable() }),
    mPreviewState(PreviewState::none),
    mPreviewChannel(trackerboy::ChType::ch1),
    mRendering(false),
    mMusicPlaying(false),
    QObject()
{
}

void RenderWorker::previewInstrument(trackerboy::Note note) {
    QMutexLocker locker(&mMutex);

    if (mPreviewState == PreviewState::instrument) {
        mIr.playNote(note);
    } else {
        auto inst = mInstrumentModel.instrument(mInstrumentModel.currentIndex());
        mIr.setInstrument(*inst);
        mPreviewState = PreviewState::instrument;
        mPreviewChannel = static_cast<trackerboy::ChType>(inst->data().channel);
        mEngine.unlock(mPreviewChannel);
        mIr.playNote(note);
    }
}

void RenderWorker::previewWaveform(trackerboy::Note note) {
    QMutexLocker locker(&mMutex);

    auto &gen3 = mSynth.hardware().gen3;
    if (mPreviewState == PreviewState::none) {
        gen3.copyWave(*(mWaveModel.currentWaveform()));
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
        mSynth.setOutputEnable(mPreviewChannel, trackerboy::Gbs::TERM_BOTH, false);
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

    mPb.silence();
    mPb.start();

    for (;;) {
        mMutex.lock();
        if (mPreviewState == PreviewState::none && mMusicPlaying == false) {
            mMutex.unlock();
            break;
        }

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
        mPb.writeAll(mSynth.buffer(), framesize);

        // now add this frame to the history, so that the UI can update visualizers
        //Frame frame;
        //frame.setSamples(mSynth.buffer(), framesize);
        //mFrameHistory.enqueue(frame)

        if (stopAfterFrame) {

            mMutex.lock();
            mMusicPlaying = false;
            mMutex.unlock();
            break;
        }

    }


    mPb.stop(true);

    mMutex.lock();
    mRendering = false;
    mMutex.unlock();
}
