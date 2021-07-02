
#pragma once

#include <QMutex>
#include <QMutexLocker>

#include <utility>

//
// This class provides mutually exclusive access to a type.
// 
// Ex: RtAudio
// Guarded<RtAudio> api(RtAudio::WASAPI);
// auto handle = api.access();
// handle->openStream(...);
// handle->startStream();
// 
// handle now has exclusive access to the contained RtAudio instance in api
//
template <class T>
class Guarded {

public:
    class Handle : public QMutexLocker {
        friend class Guarded<T>;

    public:
        constexpr T* operator->() {
            return &mRef;
        }

    private:
        constexpr Handle(QMutex &mutex, T &ref) :
            QMutexLocker(&mutex),
            mRef(ref)
        {
        }

        Q_DISABLE_COPY(Handle)

        T &mRef;
    };

    template <typename... Ts>
    Guarded(Ts&&... args) :
        mHandle(std::forward<Ts>(args)...),
        mMutex()
    {
    }

    //
    // Gets access to the contained object without locking the mutex
    // Be careful using this.
    //
    T& unguarded() {
        return mHandle;
    }

    //
    // Returns a handle to the contained object that holds the mutex
    // for the lifetime of the handle.
    //
    Handle access() {
        return { mMutex, mHandle };
    }

private:
    T mHandle;
    QMutex mMutex;
};
