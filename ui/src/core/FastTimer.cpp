
#include "core/FastTimer.hpp"

#include <QMutexLocker>
#include <QTimerEvent>
#include <QThread>

constexpr auto NO_TIMER = -1;

FastTimer::FastTimer(QObject *parent) :
    QObject(parent),
    mCallback(nullptr),
    mCallbackData(nullptr),
    mInterval(0),
    mTimerType(Qt::CoarseTimer),
    mTimerId(NO_TIMER)
{
}

void FastTimer::setCallback(CallbackFn function, void *data) {
    QMutexLocker locker(&mMutex);
    mCallback = function;
    mCallbackData = data;
}

void FastTimer::setInterval(int ms, Qt::TimerType type) {
    mMutex.lock();
    bool changed = false;
    if (mInterval != ms) {
        mInterval = ms;
        changed = true;
    }
    if (mTimerType != type) {
        mTimerType = type;
        changed = true;
    }
    auto timerId = mTimerId;
    mMutex.unlock();

    if (changed && timerId != NO_TIMER) {
        // restart the timer with the new interval
        start();
    }
}

void FastTimer::timerEvent(QTimerEvent *evt) {
    mMutex.lock();
    auto const timerId = mTimerId;
    auto const callback = mCallback;
    auto const data = mCallbackData;
    mMutex.unlock();


    if (evt->timerId() == timerId && callback) {
        callback(data);
    }

}

bool FastTimer::isThreadSafe() const {
    return thread() == QThread::currentThread();
}


void FastTimer::start() {
    if (isThreadSafe()) {
        QMutexLocker locker(&mMutex);
        _stopTimer();
        _startTimer();
    } else {
        // re-invoke in the object's thread
        QMetaObject::invokeMethod(this, [this]() { start(); }, Qt::BlockingQueuedConnection);
    }
}

void FastTimer::stop() {
    if (isThreadSafe()) {
        QMutexLocker locker(&mMutex);
        _stopTimer();
    } else {
        QMetaObject::invokeMethod(this, [this]() { stop(); }, Qt::BlockingQueuedConnection);
    }
}


void FastTimer::_startTimer() {
    mTimerId = startTimer(mInterval, mTimerType);
}

void FastTimer::_stopTimer() {
    if (mTimerId != NO_TIMER) {
        killTimer(mTimerId);
        mTimerId = -1;
    }
}
