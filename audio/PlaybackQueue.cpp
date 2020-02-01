
#include "audio.hpp"

#include <algorithm>
#include <cmath>

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
        std::fill_n(out + (static_cast<size_t>(nread) * 2), (frameCount - nread) * 2, 0);
    }

    //if (PaUtil_GetRingBufferReadAvailable(&pb->mQueue) == 0) {
        // buffer is now empty, stop the stream
    //    return paComplete;
    //} else {
        return paContinue;
    //}
}

PlaybackQueue::PlaybackQueue(float samplingRate, unsigned bufferSize) :
    mStream(nullptr),
    mSamplingRate(samplingRate),
    mBufferSize(bufferSize)
{
    checkSamplingRate(samplingRate);
    checkBufferSize(bufferSize);

    // resize the queue vector
    resizeQueue();

    // open the stream
    openStream();

}

PlaybackQueue::~PlaybackQueue() {
    if (mStream != nullptr) {
        Pa_CloseStream(mStream);
    }
}

size_t PlaybackQueue::bufferSampleSize() {
    // divide by two since 1 sample is two shorts
    return (mQueueData.size() / 2) - mSlack;
}

unsigned PlaybackQueue::bufferSize() {
    return mBufferSize;
}


bool PlaybackQueue::canWrite(size_t nsamples) {
    return  PaUtil_GetRingBufferWriteAvailable(&mQueue) - mSlack >= nsamples;
}

void PlaybackQueue::flush() {
    PaUtil_FlushRingBuffer(&mQueue);
}

void PlaybackQueue::setBufferSize(unsigned bufferSize) {

    checkBufferSize(bufferSize);

    if (mStream != nullptr && Pa_IsStreamActive(mStream)) {
        throw std::runtime_error("cannot change buffer size while stream is active");
    }

    flush();
    mBufferSize = bufferSize;
    resizeQueue();
}


void PlaybackQueue::setSamplingRate(float samplingRate) {

    checkSamplingRate(samplingRate);

    if (mStream != nullptr) {
        if (Pa_IsStreamActive(mStream)) {
            return; // cannot change the samping rate while the stream is active
        }
        // close the existing stream, portaudio doesn't have a function for
        // changing the sampling rate, so we have to open a new stream
        Pa_CloseStream(mStream);
    }

    flush();
    mSamplingRate = samplingRate;
    openStream();
    resizeQueue();
}

void PlaybackQueue::start() {
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
        const size_t remainingSamples = queueSize - PaUtil_GetRingBufferWriteAvailable(&mQueue);
        // determine how long to sleep until the queue is read out
        unsigned sleepTime = static_cast<unsigned>(std::round(
            (mSamplingRate / (remainingSamples * 1000.0f))
        ));

        Pa_Sleep(sleepTime);

        while (PaUtil_GetRingBufferWriteAvailable(&mQueue) != queueSize) {
            // just spin lock until the stream ends
            Pa_Sleep(10);
        }
    } 
    PaError err = Pa_StopStream(mStream);
    if (err != paNoError) {
        throw PaException(err);
    }

}

size_t PlaybackQueue::write(int16_t buf[], size_t nsamples) {

    size_t navail = PaUtil_GetRingBufferWriteAvailable(&mQueue) - mSlack;
    size_t samplesToWrite = nsamples > navail ? navail : nsamples;
    return PaUtil_WriteRingBuffer(&mQueue, buf, samplesToWrite);
}

// private methods

void PlaybackQueue::checkBufferSize(unsigned bufferSize) {
    if (bufferSize < MIN_BUFFER_SIZE || bufferSize > MAX_BUFFER_SIZE) {
        throw std::invalid_argument("buffer size is out of range");
    }
}

void PlaybackQueue::checkSamplingRate(float samplingRate) {
    if (samplingRate <= 0.0f) {
        throw std::invalid_argument("sampling rate must be positive");
    }
}

void PlaybackQueue::openStream() {
    PaError err = Pa_OpenDefaultStream(
        &mStream,                       // the stream pointer
        0,                              // no input channels
        2,                              // stereo, 2 output channels
        paInt16,                        // 16-bit integer samples
        mSamplingRate,                  // use the set sampling rate
        paFramesPerBufferUnspecified,   // use the best fpb for the host
        playbackCallback,               // callback function
        this                            // pass this to the callback function
    );

    if (err != paNoError) {
        throw PaException(err);
    }
}

void PlaybackQueue::resizeQueue() {
    
    // determine the number of samples the queue requires
    const size_t nsamples = static_cast<size_t>(std::round(mSamplingRate * (mBufferSize / 1000.0f)));
    
    // find the nearest power of two that is >= nsamples
    // there's a faster way to do this but performance is not a concern here.

    size_t queueDataSize = 1;
    while (queueDataSize < nsamples) {
        queueDataSize <<= 1;
    }

    // resize the queue vector
    mQueueData.resize(queueDataSize * 2);

    // re-initialize ringbuffer with the new size
    PaUtil_InitializeRingBuffer(&mQueue, sizeof(int16_t) * 2, queueDataSize, mQueueData.data());
    
    // update the slack
    // worst case: slack = nsamples+1 (~50% unused)
    // best case: slack = 0 (0% unused)
    mSlack = queueDataSize - nsamples;
    
}


}
