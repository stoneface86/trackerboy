
#include "audio.hpp"

#include <algorithm>

static constexpr ring_buffer_size_t N_FRAMES = 32768;
static constexpr float FRAMERATE = 59.7f;

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

    auto nread = PaUtil_ReadRingBuffer(&pb->ringbuf, out, frameCount);
    if (nread != frameCount) {
        std::fill_n(out + nread, frameCount - nread, 0.0f);
    }

    if (PaUtil_GetRingBufferReadAvailable(&pb->ringbuf) == 0) {
        // buffer is now empty, stop the stream
        return paComplete;
    } else {
        return paContinue;
    }
}

PlaybackQueue::PlaybackQueue(float samplingRate) :
    stream(nullptr),
    framedata(new float[N_FRAMES * 2])
{
    setSamplingRate(samplingRate);
    PaUtil_InitializeRingBuffer(&ringbuf, sizeof(float) * 2, N_FRAMES, framedata.get());
}

PlaybackQueue::~PlaybackQueue() {
    if (stream != nullptr) {
        Pa_CloseStream(stream);
    }
}

bool PlaybackQueue::canWrite() {
    return PaUtil_GetRingBufferWriteAvailable(&ringbuf) >= samplesPerFrame;
}

size_t PlaybackQueue::framesize() {
    return samplesPerFrame;
}


void PlaybackQueue::setSamplingRate(float _samplingRate) {
    samplingRate = _samplingRate;
    if (stream != nullptr) {
        if (Pa_IsStreamActive(stream)) {
            return; // cannot change the samping rate while the stream is active
        }

        Pa_CloseStream(stream);
    }

    samplesPerFrame = static_cast<size_t>(samplingRate / FRAMERATE);

    PaError err = Pa_OpenDefaultStream(
        &stream,
        0,
        2,
        paFloat32,
        samplingRate,
        paFramesPerBufferUnspecified,
        playbackCallback,
        this
    );

    if (err != paNoError) {
        throw PaException(err);
    }
}

void PlaybackQueue::start() {
    PaUtil_FlushRingBuffer(&ringbuf);
    PaError err = Pa_StartStream(stream);
    if (err != paNoError) {
        throw PaException(err);
    }
}

void PlaybackQueue::stop(bool wait) {
    // wait until all frames have been played out
    if (wait) {
        while (PaUtil_GetRingBufferWriteAvailable(&ringbuf) != N_FRAMES) {
            Pa_Sleep(20);
        }
    }

    PaError err = Pa_StopStream(stream);
    if (err != paNoError) {
        throw PaException(err);
    }

    if (!wait) {
        PaUtil_FlushRingBuffer(&ringbuf);
    }
}

void PlaybackQueue::writeFrame(float frame[]) {
    PaUtil_WriteRingBuffer(&ringbuf, frame, samplesPerFrame);
    if (!Pa_IsStreamActive(stream)) {
        PaError err = Pa_StartStream(stream);
        if (err != paNoError) {
            throw PaException(err);
        }
    }
}


}