#pragma once

#include <QMutex>
#include <QMutexLocker>

//
// QMutexLocker subclass holding a reference to a locked object.
//
template <class T>
class Locked : public QMutexLocker<QMutex> {

    T &mRef;

public:
    constexpr Locked(T &ref, QMutex &mutex) :
        QMutexLocker(&mutex),
        mRef(ref)
    {
    }

    constexpr T* operator->() {
        return &mRef;
    }

private:
    // disable copy semantics, as copying a QMutexLocker makes no sense
    Q_DISABLE_COPY(Locked)

};
