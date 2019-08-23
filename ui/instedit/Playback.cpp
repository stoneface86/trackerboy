
#include "Playback.hpp"

#include <algorithm>

static constexpr ring_buffer_size_t N_FRAMES = 32768;
static constexpr float FRAMERATE = 59.7f;

namespace instedit {

int playbackCallback(
    const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData
) {
    (void)input;
    float *outLeft = static_cast<float**>(output)[0];
    float *outRight = static_cast<float**>(output)[1];
    Playback *pb = static_cast<Playback*>(userData);

    // read 1 frame from the ring buffers
    auto nread = PaUtil_ReadRingBuffer(&pb->ringLeft, outLeft, frameCount);
    if (nread != frameCount) {
        std::fill_n(outLeft + nread, frameCount - nread, 0.0f);
    }
    nread = PaUtil_ReadRingBuffer(&pb->ringRight, outRight, frameCount);
    if (nread != frameCount) {
        std::fill_n(outRight + nread, frameCount - nread, 0.0f);
    }
    
    return paContinue;
}

Playback::Playback(float samplingRate) :
    stream(nullptr),
    framedataLeft(new float[N_FRAMES]),
    framedataRight(new float[N_FRAMES])
{
    setSamplingRate(samplingRate);
    PaUtil_InitializeRingBuffer(&ringLeft, sizeof(float), N_FRAMES, framedataLeft.get());
    PaUtil_InitializeRingBuffer(&ringRight, sizeof(float), N_FRAMES, framedataRight.get());
}

Playback::~Playback() {
    if (stream != nullptr) {
        Pa_CloseStream(stream);
    }
}

bool Playback::canWrite() {
    return PaUtil_GetRingBufferWriteAvailable(&ringRight) >= samplesPerFrame;
}

size_t Playback::framesize() {
    return samplesPerFrame;
}


void Playback::setSamplingRate(float _samplingRate) {
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
        paFloat32 | paNonInterleaved,
        samplingRate,
        paFramesPerBufferUnspecified,
        playbackCallback,
        this
    );

    if (err != paNoError) {
        throw PaException(err);
    }
}

void Playback::start() {
    PaUtil_FlushRingBuffer(&ringLeft);
    PaUtil_FlushRingBuffer(&ringRight);
    PaError err = Pa_StartStream(stream);
    if (err != paNoError) {
        throw PaException(err);
    }
}

void Playback::stop(bool wait) {
    // wait until all frames have been played out
    if (wait) {
        while (PaUtil_GetRingBufferWriteAvailable(&ringRight) != N_FRAMES) {
            Pa_Sleep(20);
        }
    }

    PaError err = Pa_StopStream(stream);
    if (err != paNoError) {
        throw PaException(err);
    }
}

void Playback::writeFrame(float framesLeft[], float framesRight[]) {

    PaUtil_WriteRingBuffer(&ringLeft, framesLeft, samplesPerFrame);
    PaUtil_WriteRingBuffer(&ringRight, framesRight, samplesPerFrame);

}


}