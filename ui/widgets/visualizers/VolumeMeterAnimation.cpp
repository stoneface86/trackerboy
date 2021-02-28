
#include "widgets/visualizers/VolumeMeterAnimation.hpp"

#include <QtDebug>

constexpr int DURATION = 100;

VolumeMeterAnimation::VolumeMeterAnimation(QObject *parent) :
    QAbstractAnimation(parent),
    mTargetSample(0),
    mTarget(MIN_DB),
    mVolume(MIN_DB),
    mStartValue(MIN_DB),
    mDifference(0),
    mCurve(QEasingCurve::OutSine)
{
}

qreal VolumeMeterAnimation::volume() const {
    return mVolume;
}

qreal VolumeMeterAnimation::meterWidth(int width) {
    if (mVolume < MIN_DB) {
        return 0.0;
    } else {
        return (mVolume - MIN_DB) * width / -MIN_DB;
    }
}

int VolumeMeterAnimation::duration() const {
    // duration is indefinite, animation runs until the target volume is reached
    return DURATION;
}

void VolumeMeterAnimation::setTarget(qint16 volume) {
    if (mTargetSample != volume) {
        mTargetSample = volume;
        auto target = volume ? 6.0 * log2(volume / (double)INT16_MAX) : MIN_DB;
        if (target < MIN_DB) {
            target = MIN_DB;
        }

        if (!qFuzzyCompare(target, mTarget)) {
            stop();
            mTarget = target;
            mStartValue = mVolume;
            mDifference = mTarget - mVolume;
            start();
        }
    }
    
}

void VolumeMeterAnimation::updateCurrentTime(int currentTime) {
    mVolume = mStartValue + (mCurve.valueForProgress(currentTime / (qreal)DURATION) * mDifference);
    emit redraw();
}
