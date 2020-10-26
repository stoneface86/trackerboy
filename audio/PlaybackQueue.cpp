
#include "audio.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <thread>
#include <chrono>
#include <stdexcept>

namespace audio {

size_t readRb(ma_pcm_rb *ringbuffer, int16_t *buf, size_t frames) {
    int16_t *src;
    ma_uint32 framesToRead = frames;
    ma_pcm_rb_acquire_read(ringbuffer, &framesToRead, reinterpret_cast<void**>(&src));
    {
        size_t samplesToRead = framesToRead * 2;
        // copy samples from the read pointer to the buffer
        std::copy_n(src, samplesToRead, buf);
    }
    ma_pcm_rb_commit_read(ringbuffer, framesToRead, src);
    return framesToRead;
}

size_t writeRb(ma_pcm_rb *ringbuffer, int16_t *buf, size_t frames) {
    int16_t *dest;
    ma_uint32 framesToWrite = frames;
    ma_pcm_rb_acquire_write(ringbuffer, &framesToWrite, reinterpret_cast<void**>(&dest));
    {
        size_t samplesToWrite = framesToWrite * 2;
        // copy samples from the buffer to the write pointer
        std::copy_n(buf, samplesToWrite, dest);
    }
    ma_pcm_rb_commit_write(ringbuffer, framesToWrite, dest);
    return framesToWrite;
}

static size_t wrappedRead(ma_pcm_rb *ringbuffer, int16_t *buf, size_t frames) {

    auto readAvail = ma_pcm_rb_available_read(ringbuffer);

    if (readAvail == 0) {
        return 0;
    }

    // do the first read
    size_t framesRead = readRb(ringbuffer, buf, frames);

    frames -= framesRead;
    readAvail -= framesRead;

    if (frames != 0 && readAvail) {
        // do the second read (the read pointer starts at the beginning)
        return framesRead + readRb(ringbuffer, buf + (framesRead * 2), frames);

    }

    return framesRead;
}

// writes to a ma_pcm_rb and wraps back to start if at end of buffer
// before write:
// +------+--------+-----+
// |      | xxxxxx |     |
// +------+--------+-----+
//        R        W
// after
// +---+--+--------------+
// | y |  | xxxxxxxyyyyy |
// +---+--+--------------+
//     W  R
//
static size_t wrappedWrite(ma_pcm_rb *ringbuffer, int16_t *buf, size_t frames) {

    auto writeAvail = ma_pcm_rb_available_write(ringbuffer);

    if (writeAvail == 0) {
        return 0;
    }

    // do the first write
    size_t framesWritten = writeRb(ringbuffer, buf, frames);

    frames -= framesWritten;
    writeAvail -= framesWritten;

    if (frames != 0 && writeAvail) {
        // do the second write (the write pointer starts at the beginning)
        return framesWritten + writeRb(ringbuffer, buf + (framesWritten * 2), frames);

    }

    return framesWritten;
}



void PlaybackQueue::playbackCallback(ma_device *device, void *out, const void *in, ma_uint32 frames) {
    (void)in;

    PlaybackQueue *pb = static_cast<PlaybackQueue*>(device->pUserData);
    auto ringbuffer = &pb->mRingbuffer.value();

    size_t framesRead = wrappedRead(ringbuffer, reinterpret_cast<int16_t*>(out), frames);

    if (pb->mQueueFull.test_and_set()) {
        ma_event_signal(&pb->mReadEvent);
        
    }
    pb->mQueueFull.clear();
    

    if (pb->mStopping) {
        if (ma_pcm_rb_available_read(ringbuffer) == 0) {
            // signal that we are done
            ma_event_signal(&pb->mStopEvent);
        }
    } else if (framesRead != frames) {
        // underrun! just increment the counter for diagnostic purposes
        ++pb->mUnderruns;
    }
}

PlaybackQueue::PlaybackQueue() :
    mSamplerate(DEFAULT_SAMPLERATE),
    mBufferSize(DEFAULT_BUFFERSIZE),
    mResizeRequired(true),
    mRunning(false),
    mUnderruns(0),
    mStopping(false)
{
    ma_event_init(&mReadEvent);
    ma_event_init(&mStopEvent);
    mQueueFull.clear();
}

PlaybackQueue::~PlaybackQueue() {

    close();
    if (mRingbuffer) {
        ma_pcm_rb_uninit(&mRingbuffer.value());
    }
    ma_event_uninit(&mReadEvent);
    ma_event_uninit(&mStopEvent);

}


void PlaybackQueue::open() {

    if (!mDevice) {
        auto config = ma_device_config_init(ma_device_type_playback);
        config.playback.format = ma_format_s16;
        config.playback.channels = 2;
        config.sampleRate = mSamplerate;
        config.dataCallback = playbackCallback;
        config.pUserData = this;

        mDevice.emplace();
        auto err = ma_device_init(nullptr, &config, &mDevice.value());
        assert(err == MA_SUCCESS);

        if (mResizeRequired) {
            mResizeRequired = false;
            if (mRingbuffer) {
                ma_pcm_rb_uninit(&mRingbuffer.value());
                mRingbuffer.reset();
            }
            const size_t nsamples = static_cast<size_t>(
                round(mDevice.value().sampleRate * (mBufferSize * 0.001f))
                );
            mRingbuffer.emplace();
            auto err = ma_pcm_rb_init(ma_format_s16, 2, nsamples, nullptr, nullptr, &mRingbuffer.value());
            assert(err == MA_SUCCESS);

        }
    }
}

void PlaybackQueue::close() {
    if (mDevice) {
        ma_device_uninit(&mDevice.value());
        mDevice.reset();
    }
}

void PlaybackQueue::start() {
    if (!mRunning) {
        auto err = ma_device_start(&mDevice.value());
        assert(err == MA_SUCCESS);
        mRunning = true;
    }
}

void PlaybackQueue::setBufferSize(unsigned buffersize) {
    mBufferSize = buffersize;
    mResizeRequired = true;
}

void PlaybackQueue::setSamplerate(unsigned samplerate) {
    mSamplerate = samplerate;
    mResizeRequired = true;
}

void PlaybackQueue::stop(bool wait) {
    if (mDevice) {
        if (wait) {
            mStopping = true;
            start();
            ma_event_wait(&mStopEvent);
        }

        auto err = ma_device_stop(&mDevice.value());
        assert(err == MA_SUCCESS);
        mRunning = false;
    }
}

void PlaybackQueue::enqueue(int16_t buf[], size_t nsamples) {
    if (!mDevice) {
        throw std::runtime_error("Cannot enqueue samples: device not opened");
    }

    auto src = buf;
    size_t toWrite = nsamples;
    size_t nwritten = 0;
    auto ringbuffer = &mRingbuffer.value();
    for (;;) {
        nwritten = wrappedWrite(ringbuffer, src, toWrite);
        if (nwritten == toWrite) {
            break;
        }
        start(); // start playback if not already started, does nothing if already started

        toWrite -= nwritten;

        mQueueFull.test_and_set();
        ma_event_wait(&mReadEvent);

        src += nwritten * 2;

    }
}

unsigned PlaybackQueue::underruns() const noexcept {
    return mUnderruns.load();
}

void PlaybackQueue::resetUnderruns() noexcept {
    mUnderruns = 0;
}

}
