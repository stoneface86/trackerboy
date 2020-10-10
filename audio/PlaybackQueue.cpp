
#include "audio.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <chrono>
#include <thread>

namespace audio {

int playbackCallback(
    const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData
) {
    (void)input;
    int16_t *out = static_cast<int16_t*>(output);
    PlaybackQueue *pb = static_cast<PlaybackQueue*>(userData);

    auto nread = PaUtil_ReadRingBuffer(&pb->mQueue, out, frameCount);
    if (nread != frameCount) {
        std::fill_n(out + (static_cast<size_t>(nread) * 2), (frameCount - nread) * 2, static_cast<int16_t>(0));
    }
    
    return PaUtil_GetRingBufferReadAvailable(&pb->mQueue) ? paContinue : paComplete;
    
}

PlaybackQueue::PlaybackQueue(Samplerate samplerate, unsigned bufferSize) :
    mStream(nullptr),
    mSamplerate(samplerate),
    mBufferSize(bufferSize),
    mResizeRequired(true),
    mDevice(Pa_GetDefaultOutputDevice()),
    mWaitTime(0)
{
    checkBufferSize(bufferSize);
}

PlaybackQueue::~PlaybackQueue() {
    //close();
}

size_t PlaybackQueue::bufferSampleSize() {
    // divide by two since 1 sample is two shorts
    return (mQueueData.size() / 2);
}

unsigned PlaybackQueue::bufferSize() {
    return mBufferSize;
}


bool PlaybackQueue::canWrite(size_t nsamples) {
    return  PaUtil_GetRingBufferWriteAvailable(&mQueue) >= nsamples;
}

void PlaybackQueue::close() {
    if (mStream != nullptr) {
        if (Pa_IsStreamActive(mStream) == 1) {
            stop(false);
        }
        Pa_CloseStream(mStream);
        mStream = nullptr;
    }
}

void PlaybackQueue::open() {
    if (mStream == nullptr) {
        PaStreamParameters param;
        param.channelCount = 2;
        param.device = mDevice;
        param.hostApiSpecificStreamInfo = nullptr;
        param.sampleFormat = paInt16;
        auto info = Pa_GetDeviceInfo(mDevice);
        if (info == nullptr) {
            throw std::runtime_error("cannot open stream: unknown device id");
        }
        param.suggestedLatency = info->defaultLowOutputLatency;

        float samplerate = SAMPLERATE_TABLE[mSamplerate];

        PaError err = Pa_OpenStream(
            &mStream,                       // the stream pointer
            NULL,                           // no input channels
            &param,                         // stereo, 2 output channels
            samplerate,                     // use the set sampling rate
            paFramesPerBufferUnspecified,   // use the best fpb for the host
            paNoFlag,                       // stream flags: none
            playbackCallback,               // callback function
            this                            // pass this to the callback function
        );

        if (err != paNoError) {
            throw PaException(err);
        }

        // time to wait when writeAll or stop blocks
        mWaitTime = mBufferSize / 4;
    }

    if (mResizeRequired) {
        // determine the number of samples the queue requires
        const size_t nsamples = static_cast<size_t>(std::round(SAMPLERATE_TABLE[mSamplerate] * (mBufferSize / 1000.0f)));

        // find the nearest power of two that is >= nsamples
        // there's a faster way to do this but performance is not a concern here.

        size_t queueDataSize = 1;
        while (queueDataSize < nsamples) {
            queueDataSize <<= 1;
        }
        assert(((queueDataSize - 1) & queueDataSize) == 0);

        // resize the queue vector
        mQueueData.resize(queueDataSize * 2);

        // re-initialize ringbuffer with the new size
        PaUtil_InitializeRingBuffer(&mQueue, sizeof(int16_t) * 2, queueDataSize, mQueueData.data());

        mResizeRequired = false;
    }
}

void PlaybackQueue::setBufferSize(unsigned bufferSize) {

    checkBufferSize(bufferSize);

    mBufferSize = bufferSize;
    mResizeRequired = true;
}

void PlaybackQueue::setDevice(int deviceId, Samplerate samplerate) {
    if (Pa_GetDeviceInfo(deviceId) == nullptr) {
        throw std::invalid_argument("cannot set device: unknown id");
    }
    // user must close and reopen stream for changes to take effect
    mDevice = deviceId;
    if (samplerate != mSamplerate) {
        mSamplerate = samplerate;
        // if the samplerate changes then we will need to resize the buffer
        mResizeRequired = true;
    }
}

void PlaybackQueue::stop(bool wait) {
    // wait until all samples have been played out
    if (wait) {
        if (Pa_IsStreamActive(mStream) == 0) {
            // stream was never started, start it just to play out the queue
            auto error = Pa_StartStream(mStream);
        }
        const size_t queueSize = mQueueData.size() / 2;

        while (Pa_IsStreamActive(mStream) == 1) {
            // just sleep until the stream ends
            auto remaining = queueSize - PaUtil_GetRingBufferWriteAvailable(&mQueue);
            std::this_thread::sleep_for(std::chrono::milliseconds(
                static_cast<unsigned>(remaining * 1000 / SAMPLERATE_TABLE[mSamplerate])
            ));
        }
        Pa_StopStream(mStream);
        PaUtil_FlushRingBuffer(&mQueue);
    } else {

        // force stop and flush buffer
        if (Pa_IsStreamActive(mStream)) {
            PaError err = Pa_StopStream(mStream);
            if (err != paNoError) {
                throw PaException(err);
            }
            PaUtil_FlushRingBuffer(&mQueue);
        }
    }

}

size_t PlaybackQueue::write(int16_t buf[], size_t nsamples) {

    size_t navail = PaUtil_GetRingBufferWriteAvailable(&mQueue);
    size_t samplesToWrite = nsamples > navail ? navail : nsamples;
    PaUtil_WriteRingBuffer(&mQueue, buf, samplesToWrite);
    if (nsamples >= navail) {
        // queue is full, start the stream
        if (!Pa_IsStreamActive(mStream)) {
            PaError error = Pa_StartStream(mStream);
            if (error != paNoError) {
                throw PaException(error);
            }
        }
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
        std::this_thread::sleep_for(std::chrono::milliseconds(
            mWaitTime
            //static_cast<unsigned>(toWrite * mSleepFactor)
        ));
        
        fp += nwritten * 2;
    }
}

// private methods

void PlaybackQueue::checkBufferSize(unsigned bufferSize) {
    if (bufferSize < MIN_BUFFER_SIZE || bufferSize > MAX_BUFFER_SIZE) {
        throw std::invalid_argument("buffer size is out of range");
    }
}


}
