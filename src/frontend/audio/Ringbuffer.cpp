
#include "audio/Ringbuffer.hpp"

#include "miniaudio.h"

#include <QtGlobal>

#include <cstring>


RingbufferBase::RingbufferBase() :
    mInitialized(false),
    mRingbuffer(),
    mSize(0)
{

}

RingbufferBase::~RingbufferBase() {
    uninit();
}

void RingbufferBase::init(size_t buffersize, void *buffer) {
    uninit();
    auto result = ma_rb_init(buffersize, buffer, nullptr, &mRingbuffer);
    mInitialized = result == MA_SUCCESS;
    if (mInitialized) {
        ma_rb_reset(&mRingbuffer);
    }
    mSize = buffersize;
}

void RingbufferBase::uninit() {
    if (mInitialized) {
        ma_rb_uninit(&mRingbuffer);
        mInitialized = false;
    }
}

size_t RingbufferBase::size() const {
    return mSize;
}

size_t RingbufferBase::read(void *data, size_t sizeInBytes) {
    size_t bytesToRead = sizeInBytes;
    void *src;
    
    auto result = ma_rb_acquire_read(&mRingbuffer, &bytesToRead, &src);
    Q_ASSERT(result == MA_SUCCESS);

    memcpy(data, src, bytesToRead);
    
    result = ma_rb_commit_read(&mRingbuffer, bytesToRead);
    Q_ASSERT(result == MA_SUCCESS || result == MA_AT_END);
    
    Q_UNUSED(result)
    return bytesToRead;
}

size_t RingbufferBase::write(void const *data, size_t sizeInBytes) {
    void *dest;
    size_t bytesToWrite = sizeInBytes;

    auto result = ma_rb_acquire_write(&mRingbuffer, &bytesToWrite, &dest);
    Q_ASSERT(result == MA_SUCCESS);

    memcpy(dest, data, bytesToWrite);
    
    result = ma_rb_commit_write(&mRingbuffer, bytesToWrite);
    Q_ASSERT(result == MA_SUCCESS || result == MA_AT_END);
    
    Q_UNUSED(result)
    return bytesToWrite;
}

size_t RingbufferBase::fullRead(void *buf, size_t sizeInBytes) {
    size_t readAvail = ma_rb_available_read(&mRingbuffer);

    if (readAvail == 0) {
        return 0;
    }

    // do the first read
    size_t bytesRead = read(buf, sizeInBytes);

    sizeInBytes -= bytesRead;
    readAvail -= bytesRead;

    if (sizeInBytes != 0 && readAvail) {
        // do the second read (the read pointer starts at the beginning)
        bytesRead += read(reinterpret_cast<uint8_t*>(buf) + bytesRead, sizeInBytes);

    }

    return bytesRead;
}

size_t RingbufferBase::fullWrite(void const *buf, size_t sizeInBytes) {
    size_t writeAvail = ma_rb_available_write(&mRingbuffer);

    if (writeAvail == 0) {
        return 0;
    }

    size_t bytesWritten = write(buf, sizeInBytes);

    sizeInBytes -= bytesWritten;
    writeAvail -= bytesWritten;

    if (sizeInBytes != 0 && writeAvail) {
        bytesWritten += write(reinterpret_cast<uint8_t const*>(buf) + bytesWritten, sizeInBytes);
    }

    return bytesWritten;
}

void* RingbufferBase::acquireRead(size_t &outSize) {
    void *buf;
    auto result = ma_rb_acquire_read(&mRingbuffer, &outSize, &buf);
    Q_ASSERT(result == MA_SUCCESS);
    Q_UNUSED(result)
    return buf;
}

void RingbufferBase::commitRead(size_t size) {
    auto result = ma_rb_commit_read(&mRingbuffer, size);
    Q_ASSERT(result == MA_SUCCESS);
    Q_UNUSED(result)
}

void* RingbufferBase::acquireWrite(size_t &outSize) {
    void *buf;
    auto result = ma_rb_acquire_write(&mRingbuffer, &outSize, &buf);
    Q_ASSERT(result == MA_SUCCESS);
    Q_UNUSED(result)
    return buf;
}

void RingbufferBase::commitWrite(size_t size) {
    auto result = ma_rb_commit_write(&mRingbuffer, size);
    Q_ASSERT(result == MA_SUCCESS);
    Q_UNUSED(result)
}

size_t RingbufferBase::availableRead() {
    return ma_rb_available_read(&mRingbuffer);
}

size_t RingbufferBase::availableWrite() {
    return ma_rb_available_write(&mRingbuffer);
}

void RingbufferBase::seekRead(size_t bytes) {
    auto result = ma_rb_seek_read(&mRingbuffer, bytes);
    Q_ASSERT(result == MA_SUCCESS);
    Q_UNUSED(result)
}

void RingbufferBase::seekWrite(size_t bytes) {
    auto result = ma_rb_seek_write(&mRingbuffer, bytes);
    Q_ASSERT(result == MA_SUCCESS);
    Q_UNUSED(result)
}

void RingbufferBase::reset() {
    ma_rb_reset(&mRingbuffer);
}
