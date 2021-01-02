
#pragma once

#include "miniaudio.h"

#include <cstddef>
#include <cstdint>

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

    size_t write(void const *buffer, size_t sizeInBytes);

    size_t read(void *buffer, size_t sizeInBytes);

    size_t fullRead(void *buffer, size_t sizeInBytes);

    size_t fullWrite(void const *buffer, size_t sizeInBytes);
    
private:

    bool mInitialized;
    ma_rb mRingbuffer;

    size_t mSize;

};

// thin template idiom

template <typename T, size_t channels = 1>
class Ringbuffer : public RingbufferBase {
    static_assert(channels > 0, "channels cannot be 0");
    static constexpr auto SIZE_UNIT = sizeof(T) * channels;

public:

    constexpr void init(size_t count, T* buffer = nullptr) {
        RingbufferBase::init(count * SIZE_UNIT, buffer);
    }

    constexpr size_t read(T *data, size_t count) {
        return RingbufferBase::read(data, count * SIZE_UNIT) / SIZE_UNIT;
    }

    constexpr size_t write(T const *data, size_t count) {
        return RingbufferBase::write(data, count * SIZE_UNIT) / SIZE_UNIT;
    }

    constexpr size_t fullRead(T *data, size_t count) {
        return RingbufferBase::fullRead(data, count * SIZE_UNIT) / SIZE_UNIT;
    }

    constexpr size_t fullWrite(T const *data, size_t count) {
        return RingbufferBase::fullWrite(data, count * SIZE_UNIT) / SIZE_UNIT;
    }

};

//
// ringbuffer typedef for audio used in the application
//
using AudioRingbuffer = Ringbuffer<int16_t, 2>;
