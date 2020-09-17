
#include "audio.hpp"

#include <algorithm>
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
    float *out = static_cast<float*>(output);
    PlaybackQueue *pb = static_cast<PlaybackQueue*>(userData);

    auto nread = PaUtil_ReadRingBuffer(&pb->mQueue, out, frameCount);
    if (nread != frameCount) {
        // underrun, just output silence
        // a gap will be noticeable in the output
        // TODO: update a counter in PlaybackQueue for total underruns
        std::fill_n(out + (static_cast<size_t>(nread) * 2), (frameCount - nread) * 2, 0.0f);
    }
    
    return paContinue;
    
}

PlaybackQueue::PlaybackQueue(Samplerate samplerate, unsigned bufferSize) :
    mStream(nullptr),
    mSamplerate(samplerate),
    mBufferSize(bufferSize),
    mWaitTime(bufferSize / 4),
    mResizeRequired(true),
    mDevice(Pa_GetDefaultOutputDevice())
{
    checkBufferSize(bufferSize);
}

PlaybackQueue::~PlaybackQueue() {
    if (mStream != nullptr) {
        Pa_CloseStream(mStream);
    }
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

void PlaybackQueue::flush() {
    if (mStream != nullptr && Pa_IsStreamActive(mStream)) {
        throw std::runtime_error("cannot flush buffer while stream is active");
    }

    PaUtil_FlushRingBuffer(&mQueue);
}

void PlaybackQueue::setBufferSize(unsigned bufferSize) {

    checkBufferSize(bufferSize);

    if (mStream != nullptr && Pa_IsStreamActive(mStream)) {
        throw std::runtime_error("cannot change buffer size while stream is active");
    }

    mBufferSize = bufferSize;
    mWaitTime = bufferSize / 4;
    mResizeRequired = true;
}


void PlaybackQueue::setDevice(int deviceId) {
    if (Pa_GetDeviceInfo(deviceId) == nullptr) {
        throw std::invalid_argument("cannot set device: unknown id");
    }

    if (mStream != nullptr) {
        if (Pa_IsStreamActive(mStream)) {
            return; // cannot change device while stream is active
        }

        Pa_CloseStream(mStream);
        mStream = nullptr;
    }

    mDevice = deviceId;
}


void PlaybackQueue::setSamplingRate(Samplerate samplerate) {
    if (mStream != nullptr) {
        if (Pa_IsStreamActive(mStream)) {
            return; // cannot change the samping rate while the stream is active
        }
        // close the existing stream, portaudio doesn't have a function for
        // changing the sampling rate, so we have to open a new stream
        Pa_CloseStream(mStream);
        mStream = nullptr;
    }

    mSamplerate = samplerate;
    mResizeRequired = true;
}

void PlaybackQueue::silence() {
    std::fill(mQueueData.begin(), mQueueData.end(), 0.0f);
    PaUtil_AdvanceRingBufferWriteIndex(&mQueue, mQueueData.size() / 2);
}

void PlaybackQueue::start() {
    if (mStream == nullptr) {
        PaStreamParameters param;
        param.channelCount = 2;
        param.device = mDevice;
        param.hostApiSpecificStreamInfo = nullptr;
        param.sampleFormat = paFloat32;
        auto info = Pa_GetDeviceInfo(mDevice);
        param.suggestedLatency = info->defaultLowOutputLatency;


        PaError err = Pa_OpenStream(
            &mStream,                       // the stream pointer
            NULL,                           // no input channels
            &param,                         // stereo, 2 output channels
            SAMPLERATE_TABLE[mSamplerate],  // use the set sampling rate
            paFramesPerBufferUnspecified,   // use the best fpb for the host
            paNoFlag,                       // stream flags: none
            playbackCallback,               // callback function
            this                            // pass this to the callback function
        );

        if (err != paNoError) {
            throw PaException(err);
        }
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

        // resize the queue vector
        mQueueData.resize(queueDataSize * 2);

        // re-initialize ringbuffer with the new size
        PaUtil_InitializeRingBuffer(&mQueue, sizeof(float) * 2, queueDataSize, mQueueData.data());

        mResizeRequired = false;
    }

    flush();
    PaError err = Pa_StartStream(mStream);
    if (err != paNoError) {
        throw PaException(err);
    }
}

void PlaybackQueue::stop(bool wait) {
    // wait until all samples have been played out
    if (wait) {
        const size_t queueSize = mQueueData.size() / 2;

        while (PaUtil_GetRingBufferWriteAvailable(&mQueue) != queueSize) {
            // just spin lock until the stream ends
            std::this_thread::sleep_for(std::chrono::milliseconds(mWaitTime));
        }
    } 
    PaError err = Pa_StopStream(mStream);
    if (err != paNoError) {
        throw PaException(err);
    }

}

size_t PlaybackQueue::write(float buf[], size_t nsamples) {

    size_t navail = PaUtil_GetRingBufferWriteAvailable(&mQueue);
    size_t samplesToWrite = nsamples > navail ? navail : nsamples;
    return PaUtil_WriteRingBuffer(&mQueue, buf, samplesToWrite);
}

void PlaybackQueue::writeAll(float buf[], size_t nsamples) {
    float *fp = buf;
    size_t toWrite = nsamples;
    size_t nwritten = 0;
    for (;;) {
        nwritten = write(fp, toWrite);
        if (nwritten == toWrite) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(mWaitTime));
        toWrite -= nwritten;
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
