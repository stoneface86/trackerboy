
#pragma once

#include <QAbstractAnimation>
#include <QEasingCurve>

// this class is used internally by PeakMeter

//
// Simple animation class for a volume meter. Set the target volume
// via setTarget and the class will animate the level using sine interpolation.
//
class VolumeMeterAnimation : public QAbstractAnimation {

    Q_OBJECT

public:

    static constexpr qreal MIN_DB = -45.0;

    VolumeMeterAnimation(QObject *parent = nullptr);

    //
    // Current volume level, in dB.
    //
    qreal volume() const;

    //
    // Calculate the current meter level proportional to the given width
    // such that MIN_DB is 0.0 and 0.0 dB is width
    //
    qreal meterWidth(int width);

    int duration() const override;

    //
    // Set the target volume to move to. If the animation was stopped
    // it is started.
    //
    void setTarget(qint16 volume);

signals:
    //
    // emitted when the volume changes and should be redrawn
    //
    void redraw();

protected:

    void updateCurrentTime(int currentTime) override;

private:

    Q_DISABLE_COPY_MOVE(VolumeMeterAnimation)

    qint16 mTargetSample;

    // these values are in dB
    qreal mTarget;
    qreal mVolume;

    qreal mStartValue;
    qreal mDifference;

    QEasingCurve mCurve;

};
