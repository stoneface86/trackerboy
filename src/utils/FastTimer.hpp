
#pragma once

#include "verdigris/wobjectdefs.h"

#include <QObject>
#include <QMutex>

//
// Similar to QTimer, but uses a callback function instead of signals.
// The signal/slot mechanism has some overhead that is undesirable for a signal
// that is invoked very often (ie every 5 ms), especially when invoked via queued
// connection.
// 
// Qt Timers are used in this implementation for conveienence. Qt::PreciseTimer
// offers millisecond resolution on all platforms, which is desired. If a better
// implementation is needed, it can be implemented here (instead of using QObject timers).
// 
// All functions in this class are thread-safe.
//
class FastTimer : public QObject {

    W_OBJECT(FastTimer)

public:

    using CallbackFn = void(*)(void*);

    explicit FastTimer(QObject *parent = nullptr);
    virtual ~FastTimer() = default;

    void setCallback(CallbackFn function, void* data = nullptr);

    void setInterval(int ms, Qt::TimerType type = Qt::CoarseTimer);

    void start();

    void stop();

protected:

    virtual void timerEvent(QTimerEvent *evt) override;

private:

    void _startTimer();

    void _stopTimer();


    QMutex mMutex;
    CallbackFn mCallback;
    void *mCallbackData;
    int mInterval;
    Qt::TimerType mTimerType;

    int mTimerId;


};
