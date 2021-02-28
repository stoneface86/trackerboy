
#pragma once

#include "miniaudio.h"

#include <cstddef>
#include <cstdint>

//
// Wrapper for a miniaudio ringbuffer, ma_rb.
//
class RingbufferBase {

public:

    ~RingbufferBase();
    
    void uninit();

    void reset();

protected:
    RingbufferBase();

    void init(size_t buffersize, void* buffer = nullptr);


    // convenience "high level" read/write methods

    size_t write(void const *buffer, size_t sizeInBytes);

    size_t read(void *buffer, size_t sizeInBytes);

    size_t fullRead(void *buffer, size_t sizeInBytes);

    size_t fullWrite(void const *buffer, size_t sizeInBytes);

    // "low level" read/write access
    void* acquireRead(size_t &outSize);

    void commitRead(void *buf, size_t size);

    void* acquireWrite(size_t &outSize);

    void commitWrite(void *buf, size_t size);

    size_t availableRead();

    size_t availableWrite();

    void seekRead(size_t bytes);

    void seekWrite(size_t bytes);

    size_t size() const;

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

    // read interface for the ringbuffer
    class Reader {

        Ringbuffer<T, channels> &mRb;
    
    public:

        constexpr Reader(Ringbuffer<T, channels> &rb) :
            mRb(rb)
        {
        }

        constexpr size_t read(T *data, size_t count) {
            return mRb.read(data, count * SIZE_UNIT) / SIZE_UNIT;
        }

        constexpr size_t fullRead(T *data, size_t count) {
            return mRb.fullRead(data, count * SIZE_UNIT) / SIZE_UNIT;
        }

        constexpr T* acquireRead(size_t &outCount) {
            size_t size = outCount * SIZE_UNIT;
            auto result = static_cast<T*>(mRb.acquireRead(size));
            outCount = size / SIZE_UNIT;
            return result;
        }

        constexpr void commitRead(T *buf, size_t count) {
            mRb.commitRead(buf, count * SIZE_UNIT);
        }

        constexpr size_t availableRead() {
            return mRb.availableRead() / SIZE_UNIT;
        }

        constexpr void seekRead(size_t count) {
            mRb.seekRead(count * SIZE_UNIT);
        }

        //
        // Advance the read pointer to the write pointer, removing any existing reads
        //
        constexpr void flush() {
            seekRead(availableRead());
        }

    };

    // write interface for the ringbuffer
    class Writer {
        Ringbuffer<T, channels> &mRb;


    public:
        constexpr Writer(Ringbuffer<T, channels> &rb) :
            mRb(rb)
        {
        }

        constexpr size_t write(T const *data, size_t count) {
            return mRb.write(data, count * SIZE_UNIT) / SIZE_UNIT;
        }

        constexpr size_t fullWrite(T const *data, size_t count) {
            return mRb.fullWrite(data, count * SIZE_UNIT) / SIZE_UNIT;
        }

        constexpr T* acquireWrite(size_t &outCount) {
            size_t size = outCount * SIZE_UNIT;
            auto result = static_cast<T*>(mRb.acquireWrite(size));
            outCount = size / SIZE_UNIT;
            return result;
        }

        constexpr void commitWrite(T *buf, size_t count) {
            mRb.commitWrite(buf, count * SIZE_UNIT);
        }

        constexpr size_t availableWrite() {
            return mRb.availableWrite() / SIZE_UNIT;
        }


        constexpr void seekWrite(size_t count) {
            mRb.seekWrite(count * SIZE_UNIT);
        }

    };


    constexpr Ringbuffer() :
        RingbufferBase() 
    { 
    }

    constexpr Reader reader() {
        return { *this };
    }

    constexpr Writer writer() {
        return { *this };
    }

    constexpr void init(size_t count, T* buffer = nullptr) {
        RingbufferBase::init(count * SIZE_UNIT, buffer);
    }

    constexpr size_t size() {
        return RingbufferBase::size() / SIZE_UNIT;
    }

};

//
// ringbuffer typedef for audio used in the application
//
using AudioRingbuffer = Ringbuffer<int16_t, 2>;
