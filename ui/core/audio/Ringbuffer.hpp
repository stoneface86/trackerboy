
#pragma once

#include "miniaudio.h"

#include <cstddef>

//
// Wrapper for a miniaudio ringbuffer, ma_rb.
//
class RingbufferBase {

public:

    RingbufferBase();
    ~RingbufferBase();

    void init(size_t buffersize, void* buffer = nullptr);

    void uninit();

    size_t size() const;

    size_t write(void const *data, size_t bytes);

    size_t read(void *data, size_t bytes);

    size_t fullRead(void *data, size_t bytes);

    size_t fullWrite(void const *data, size_t bytes);
    
private:

    bool mInitialized;
    ma_rb mRingbuffer;

    size_t mSize;

};

// thin template idiom

template <class T>
class Ringbuffer : public RingbufferBase {

public:

    constexpr void init(size_t count, T* buffer = nullptr) {
        RingbufferBase::init(count * sizeof(T), buffer);
    }

    constexpr size_t read(T *data, size_t count) {
        return RingbufferBase::read(data, count * sizeof(T)) / sizeof(T);
    }

    constexpr size_t write(T const *data, size_t count) {
        return RingbufferBase::write(data, count * sizeof(T)) / sizeof(T);
    }

    constexpr size_t fullRead(T *data, size_t count) {
        return RingbufferBase::fullRead(data, count * sizeof(T)) / sizeof(T);
    }

    constexpr size_t fullWrite(T const *data, size_t count) {
        return RingbufferBase::fullWrite(data, count * sizeof(T)) / sizeof(T);
    }

};

