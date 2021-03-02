
#pragma once

#include "core/audio/Renderer.hpp"
#include "widgets/visualizers/AudioScope.hpp"

#include <QAbstractAnimation>
#include <QElapsedTimer>
#include <QTimer>

#include <chrono>
#include <cstdint>
#include <cstddef>
#include <memory>

//
// Not actually an animation, we just need to periodically read the return buffer
// and update the visualizers, which QAbstractAnimation provides via the updateCurrentTime method
// (using the animation class is more performant and accurate than using a QTimer since it
// will most likely be using vsync).
//
class SyncWorker : public QAbstractAnimation {
    Q_OBJECT

public:
    SyncWorker(Renderer &renderer, AudioScope &left, AudioScope &right);

    int duration() const override;

    void setSamplesPerFrame(size_t samples);

signals:
    void updateScopes();

    void positionChanged(QString const& pos);

    void speedChanged(QString const& speed);

    void peaksChanged(qint16 peakLeft, qint16 peakRight);

public slots:
    void onAudioStop();
    void onAudioStart();

protected:

    void updateCurrentTime(int currentTime) override;

private:

    void setPeaks(int16_t peakLeft, int16_t peakRight);

    Renderer &mRenderer;
    AudioScope &mLeftScope;
    AudioScope &mRightScope;

    QMutex mMutex;
    std::unique_ptr<int16_t[]> mSampleBuffer;
    size_t mSamplesPerFrame;

    // current volume peaks
    int16_t mPeakLeft;
    int16_t mPeakRight;

    //RenderFrame mLastFrame;

};
