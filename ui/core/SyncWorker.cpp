
#include "core/SyncWorker.hpp"

#include <QtDebug>
#include <QTimer>

//
// SyncWorker handles audio synchronization in a separate thread.
//

SyncWorker::SyncWorker(Renderer &renderer, AudioScope &leftScope, AudioScope &rightScope) :
    QObject(),
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

void SyncWorker::setSamplesPerFrame(size_t samples) {
    QMutexLocker locker(&mMutex);

    if (samples != mSamplesPerFrame) {
        mSamplesPerFrame = samples;
        mSampleBuffer.reset(new int16_t[samples * 2]);
    }
}

void SyncWorker::onAudioStart() {

}

void SyncWorker::onAudioStop() {
    // ignore any pending reads on the return buffer
    // as we are no longer processing them
    mRenderer.returnBuffer().flush();

    // clear peaks
    setPeaks(0, 0);

    // clear and update the scopes
    mLeftScope.clear();
    mRightScope.clear();

    emit updateScopes();
}

void SyncWorker::onAudioSync() {
    QMutexLocker locker(&mMutex);

    auto returnBuffer = mRenderer.returnBuffer();
    auto avail = returnBuffer.availableRead();
    auto frameCount = avail / mSamplesPerFrame;
    if (frameCount) {
        if (frameCount > 1) {
            // skip these frames
            returnBuffer.seekRead((frameCount - 1) * mSamplesPerFrame);
        }
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

    }

}


void SyncWorker::setPeaks(int16_t peakLeft, int16_t peakRight) {
    if (mPeakLeft != peakLeft || mPeakRight != peakRight) {
        mPeakLeft = peakLeft;
        mPeakRight = peakRight;
        emit peaksChanged(peakLeft, peakRight);
    }
}
