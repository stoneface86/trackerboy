
#pragma once

#include "core/Locked.hpp"

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

    template <typename... Ts>
    Guarded(Ts&&... args) :
        mHandle(std::forward<Ts>(args)...),
        mMutex()
    {
    }

    //
    // Returns a handle to the contained object that holds the mutex
    // for the lifetime of the handle.
    //
    Locked<T> access() {
        return { mHandle, mMutex };
    }

private:
    T mHandle;
    QMutex mMutex;
};




