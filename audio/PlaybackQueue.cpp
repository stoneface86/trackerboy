
#include "audio.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <chrono>
#include <thread>

namespace audio {

void PlaybackQueue::playbackCallback(struct SoundIoOutStream *stream, int framesMin, int framesMax) {

    (void)framesMin;

    PlaybackQueue *queue = static_cast<PlaybackQueue*>(stream->userdata);
    auto ringbuffer = queue->mRingbuffer;
    struct SoundIoChannelArea *areas;

    int framesLeft = framesMax;
    do {
        // how many samples can we read from the ringbuffer?
        int samplesRemaining = soundio_ring_buffer_fill_count(ringbuffer) / (2 * sizeof(int16_t));
        if (samplesRemaining == 0) {
            // no more samples, stop
            // this will underrun if we haven't written to framesMin yet
            break;
        }

        // try to write everything in the ringbuffer
        int framesToWrite = std::min(samplesRemaining, framesLeft);
        int err = soundio_outstream_begin_write(stream, &areas, &framesToWrite);
        if (err) {
            return;
        }

        int16_t *readPtr = reinterpret_cast<int16_t*>(soundio_ring_buffer_read_ptr(ringbuffer));

        // Note: samples in the ringbuffer are interleaved

        // Layout is ALWAYS stereo
        auto leftPtr = areas[0].ptr;
        auto leftStep = areas[0].step;
        auto rightPtr = areas[1].ptr;
        auto rightStep = areas[1].step;

        for (int i = 0; i != framesToWrite; ++i) {
            *reinterpret_cast<int16_t*>(leftPtr) = *readPtr++;
            *reinterpret_cast<int16_t*>(rightPtr) = *readPtr++;
            leftPtr += leftStep;
            rightPtr += rightStep;
        }

        soundio_ring_buffer_advance_read_ptr(ringbuffer, framesToWrite * 2 * sizeof(int16_t));

        err = soundio_outstream_end_write(stream);
        if (err) {
            return; // handle these better
        }
        framesLeft -= framesToWrite;

    } while (framesLeft);



}

void PlaybackQueue::underflowCallback(struct SoundIoOutStream *stream) {
    // just increment the underflow counter
    // this counter can be displayed to the user for diagnostic purposes

    PlaybackQueue *queue = static_cast<PlaybackQueue*>(stream->userdata);
    if (!queue->mStopping.load()) {
        queue->mUnderflowCounter++;
    }
}


PlaybackQueue::PlaybackQueue(Samplerate samplerate, unsigned bufferSize) :
    mDevice(nullptr),
    mStream(nullptr),
    mRingbuffer(nullptr),
    mSamplerate(samplerate),
    mBufferSize(bufferSize),
    mResizeRequired(true),
    mWaitTime(0),
    mStopping(false),
    mUnderflowCounter(0),
    mState(State::stopped)
{
    checkBufferSize(bufferSize);
}

PlaybackQueue::~PlaybackQueue() {
    close();
    if (mDevice != nullptr) {
        soundio_device_unref(mDevice);
    }

    if (mRingbuffer != nullptr) {
        soundio_ring_buffer_destroy(mRingbuffer);
    }
}

size_t PlaybackQueue::bufferSampleSize() {
    return soundio_ring_buffer_capacity(mRingbuffer) / (sizeof(int16_t) * 2);
}

unsigned PlaybackQueue::bufferSize() {
    return mBufferSize;
}


bool PlaybackQueue::canWrite(size_t nsamples) {
    //return  PaUtil_GetRingBufferWriteAvailable(&mQueue) >= nsamples;
    return static_cast<size_t>(soundio_ring_buffer_free_count(mRingbuffer)) >= nsamples;
}

void PlaybackQueue::close() {
    if (mStream != nullptr) {
        soundio_outstream_destroy(mStream);
        mStream = nullptr;
    }
}

void PlaybackQueue::open() {
    if (mDevice == nullptr) {
        throw std::runtime_error("cannot open stream without a device set");
    }

    if (mStream == nullptr) {

        openStream();

        // time to wait when writeAll or stop blocks
        mWaitTime = mBufferSize / 4;


        if (mResizeRequired) {
            // determine the number of samples the queue requires
            const size_t nsamples = static_cast<size_t>(
                std::round(SAMPLERATE_TABLE[mSamplerate] * (mBufferSize * 0.001f))
            );

            // no method for resizing ringbuffers so just destroy and re-create

            // the ringbuffer is null only on first open, we can't create it in the constructor
            // since it requires a SoundIo handle (but it doesn't actually need it for anything)
            if (mRingbuffer != nullptr) {
                soundio_ring_buffer_destroy(mRingbuffer);
            }

            mRingbuffer = soundio_ring_buffer_create(mDevice->soundio, nsamples * 2 * sizeof(int16_t));
            if (mRingbuffer == nullptr) {
                throw std::runtime_error("out of memory");
            }

            mResizeRequired = false;
        }
    }
}

void PlaybackQueue::setBufferSize(unsigned bufferSize) {
    if (mStream != nullptr) {
        throw std::runtime_error("cannot change buffersize while stream is open");
    }

    checkBufferSize(bufferSize);

    mBufferSize = bufferSize;
    mResizeRequired = true;
}

void PlaybackQueue::setDevice(struct SoundIoDevice *device, Samplerate samplerate) {
    // do not change settings while the stream is open
    if (mStream != nullptr) {
        throw std::runtime_error("cannot change device while stream is open");
    }

    if (device == nullptr) {
        throw std::invalid_argument("device was null");
    }

    // user must close and reopen stream for changes to take effect

    if (device != mDevice) {
        if (mDevice != nullptr) {
            soundio_device_unref(mDevice);
        }

        mDevice = device;
        soundio_device_ref(device);
    }

    if (samplerate != mSamplerate) {
        mSamplerate = samplerate;
        // if the samplerate changes then we will need to resize the buffer
        mResizeRequired = true;
    }
}

void PlaybackQueue::start() {
    if (mStream == nullptr) {
        throw std::runtime_error("cannot start stream, stream is closed");
    }

    int err = 0;
    if (mState == State::stopped) {
        err = soundio_outstream_start(mStream);
    } else if (mState == State::paused) {
        // unpause
        err = soundio_outstream_pause(mStream, false);
    }
    if (err) {
        throw SoundIoError(err);
    }
    mState = State::running;
}

void PlaybackQueue::stop(bool wait) {
    if (mStream == nullptr) {
        throw std::runtime_error("cannot stop stream, stream is closed");
    }


    if (wait) {
        // start the stream if it's never been started
        start();

        // stop counting underflows
        mStopping = true;
        // sleep so everything plays out
        std::this_thread::sleep_for(std::chrono::duration<double>(mStream->software_latency));
    }

    int err = soundio_outstream_pause(mStream, true);
    if (err) {
        if (err == SoundIoErrorIncompatibleBackend || err == SoundIoErrorIncompatibleDevice) {
            // backend or device does not support pausing
            // destroy and reopen the stream
            soundio_outstream_destroy(mStream);
            openStream();
            mState = State::stopped;
        } else {
            // stream is faulty
            throw SoundIoError(err);
        }
    } else {
        mState = State::paused;
    }
    soundio_ring_buffer_clear(mRingbuffer);
    mStopping = false;
}

size_t PlaybackQueue::write(int16_t buf[], size_t nsamples) {
    // samples available in the ringbuffer
    size_t navail = soundio_ring_buffer_free_count(mRingbuffer) / (sizeof(int16_t) * 2);
    size_t samplesToWrite = std::min(nsamples, navail);
    // get the write pointer
    auto writePtr = reinterpret_cast<int16_t*>(soundio_ring_buffer_write_ptr(mRingbuffer));
    // do the write
    std::copy_n(buf, samplesToWrite * 2, writePtr);
    soundio_ring_buffer_advance_write_ptr(mRingbuffer, samplesToWrite * sizeof(int16_t) * 2);
    if (nsamples >= navail) {
        // queue is full, start the stream
        start();

    }

    return samplesToWrite;
}

void PlaybackQueue::writeAll(int16_t buf[], size_t nsamples) {
    auto fp = buf;
    size_t toWrite = nsamples;
    size_t nwritten = 0;
    for (;;) {
        nwritten = write(fp, toWrite);
        if (nwritten == toWrite) {
            break;
        }
        toWrite -= nwritten;

        // sleep until space is available
        std::this_thread::sleep_for(std::chrono::milliseconds(mWaitTime));
        
        fp += nwritten * 2;
    }
}

unsigned PlaybackQueue::underflows() const noexcept {
    return mUnderflowCounter.load();
}

void PlaybackQueue::resetUnderflows() noexcept {
    mUnderflowCounter = 0;
}

// private methods

void PlaybackQueue::checkBufferSize(unsigned bufferSize) {
    if (bufferSize < MIN_BUFFER_SIZE || bufferSize > MAX_BUFFER_SIZE) {
        throw std::invalid_argument("buffer size is out of range");
    }
}

void PlaybackQueue::openStream() {
    mStream = soundio_outstream_create(mDevice);
    if (mStream == nullptr) {
        throw std::runtime_error("out of memory");
    }
    mStream->write_callback = playbackCallback;
    mStream->underflow_callback = underflowCallback;
    mStream->name = "trackerboy";
    mStream->software_latency = 0.001 * mBufferSize;
    mStream->sample_rate = SAMPLERATE_TABLE[mSamplerate];
    mStream->format = SoundIoFormatS16NE;
    mStream->userdata = this;

    int err = soundio_outstream_open(mStream);
    if (err) {
        throw SoundIoError(err);
    }
}


}
