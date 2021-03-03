
#include "core/SyncWorker.hpp"

#include <QtDebug>
#include <QTimer>

//
// SyncWorker handles audio synchronization in a separate thread.
//

SyncWorker::SyncWorker(Renderer &renderer, AudioScope &leftScope, AudioScope &rightScope) :
    QAbstractAnimation(),
    mRenderer(renderer),
    mLeftScope(leftScope),
    mRightScope(rightScope),
    mMutex(),
    mSampleBuffer(nullptr),
    mSamplesPerFrame(0),
    mPeakLeft(0),
    mPeakRight(0)
{
    connect(this, &SyncWorker::updateScopes, &mLeftScope, qOverload<>(&AudioScope::update), Qt::QueuedConnection);
    connect(this, &SyncWorker::updateScopes, &mRightScope, qOverload<>(&AudioScope::update), Qt::QueuedConnection);
    connect(&renderer, &Renderer::audioStarted, this, &SyncWorker::onAudioStart, Qt::QueuedConnection);
    connect(&renderer, &Renderer::audioStopped, this, &SyncWorker::onAudioStop, Qt::QueuedConnection);
    connect(&renderer, &Renderer::audioSync, this, &SyncWorker::onAudioSync, Qt::QueuedConnection);
}

int SyncWorker::duration() const {
    return -1;
}

void SyncWorker::setSamplesPerFrame(size_t samples) {
    QMutexLocker locker(&mMutex);

    if (samples != mSamplesPerFrame) {
        mSamplesPerFrame = samples;
        mSampleBuffer.reset(new int16_t[samples * 2]);
    }
}

void SyncWorker::onAudioStart() {
    //start();
}

void SyncWorker::onAudioStop() {
    stop();
    

    // ignore any pending reads on the return buffer
    // as we are no longer processing them
    mRenderer.returnBuffer().flush();

    // clear peaks
    setPeaks(0, 0);

    // clear and update the scopes
    mMutex.lock();
    mLeftScope.clear();
    mRightScope.clear();
    mMutex.unlock();

    emit updateScopes();
}

void SyncWorker::onAudioSync() {
    // if stopped and we have enough to animate a frame, start the animation
    if (state() == Stopped && mRenderer.returnBuffer().availableRead() >= mSamplesPerFrame) {
        start();
    }



}

void SyncWorker::updateCurrentTime(int currentTime) {
    Q_UNUSED(currentTime)
    
    QMutexLocker locker(&mMutex);
    Q_ASSERT(mSamplesPerFrame != 0);
    Q_ASSERT(mSampleBuffer);

    // get the audio data returned from the callback
    // this data has already been sent out to the output device
    auto returnBuffer = mRenderer.returnBuffer();
    // assert that there is enough for a frame
    Q_ASSERT(returnBuffer.availableRead() >= mSamplesPerFrame);

    // read the frame
    returnBuffer.fullRead(mSampleBuffer.get(), mSamplesPerFrame);

    // determine peak amplitudes for each channel
    int16_t peakLeft = 0;
    int16_t peakRight = 0;
    auto samplePtr = mSampleBuffer.get();
    for (size_t i = 0; i != mSamplesPerFrame; ++i) {
        auto sampleLeft = (int16_t)abs(*samplePtr++);
        auto sampleRight = (int16_t)abs(*samplePtr++);
        peakLeft = std::max(sampleLeft, peakLeft);
        peakRight = std::max(sampleRight, peakRight);
    }
    setPeaks(peakLeft, peakRight);

    // send to visualizers
    mLeftScope.render(mSampleBuffer.get(), mSamplesPerFrame);
    mRightScope.render(mSampleBuffer.get() + 1, mSamplesPerFrame);

    // calls AudioScope::update via event queue (cannot call directly as this thread is not the GUI thread)
    emit updateScopes();

    if (returnBuffer.availableRead() < mSamplesPerFrame) {
        // no more frames available, stop the animation
        stop();
    }

}

void SyncWorker::setPeaks(int16_t peakLeft, int16_t peakRight) {
    if (mPeakLeft != peakLeft || mPeakRight != peakRight) {
        mPeakLeft = peakLeft;
        mPeakRight = peakRight;
        emit peaksChanged(peakLeft, peakRight);
    }
}
