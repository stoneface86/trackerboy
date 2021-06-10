
#pragma once

#include "core/audio/Renderer.hpp"
#include "widgets/visualizers/AudioScope.hpp"

#include <chrono>
#include <cstdint>
#include <cstddef>
#include <memory>

//
// Worker object that updates visualizers, tracker position, etc, on audio sync.
//
class SyncWorker : public QObject {
    Q_OBJECT

public:
    SyncWorker(Renderer &renderer); //, AudioScope &left, AudioScope &right);

    void setSamplesPerFrame(size_t samples);

signals:
    void updateScopes();

    //
    // Tracker position is stored as a QPoint with x = pattern id, y = row
    //
    void positionChanged(QPoint const point);

    void speedChanged(QString const& speed);

    void peaksChanged(qint16 peakLeft, qint16 peakRight);

public slots:
    void onAudioStop();
    void onAudioStart();
    void onFrameSync();

private:
    Q_DISABLE_COPY(SyncWorker)

    void setPeaks(int16_t peakLeft, int16_t peakRight);

    Renderer &mRenderer;
    //AudioScope &mLeftScope;
    //AudioScope &mRightScope;

    QMutex mMutex;
    std::unique_ptr<int16_t[]> mSampleBuffer;
    size_t mSamplesPerFrame;

    // current volume peaks
    int16_t mPeakLeft;
    int16_t mPeakRight;

    RenderFrame mLastFrame;

};
