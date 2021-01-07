
#pragma once

#include "core/audio/RenderFrame.hpp"
#include "core/audio/Ringbuffer.hpp"

#include "miniaudio.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

//
// The RenderBuffer is used internally by the Renderer for audio playback buferring.
// Audio data is stored in the form of "frames" which also contain information about
// the engine and a dump of APU registers. A frame is typically 1/60 s of audio data.
//
class RenderBuffer {

public:

    RenderBuffer();
    ~RenderBuffer();

    Ringbuffer<RenderFrame>& returnFrames();

    bool isEmpty() const;

    void reset();

    //
    // Sets the total number of frames in the buffer and the minimum number of samples
    // per frame. Frame sizes range between samplesPerFrame and samplesPerFrame + 1
    //
    void setSize(unsigned frameCount, size_t samplesPerFrame);

    //
    // Number of frames that can be queued
    //
    unsigned framesToQueue();

    //
    // Number of frames queued
    //
    unsigned framesInQueue();

    unsigned size() const;


    // for the audio callback thread:

    // reads the requested number of samples to the callback output buffer
    // the amount actually read is returned
    size_t read(int16_t *out, size_t samples);

    // queues an APU frame of audio data to the buffer
    void queueFrame(int16_t data[], RenderFrame const& frame);

private:

    AudioRingbuffer mSamples;
    Ringbuffer<RenderFrame> mFrames;
    Ringbuffer<RenderFrame> mReturnFrames;

    unsigned mBuffersize;
    size_t mSamplesPerFrame;

    unsigned mFramesAvailable;

    size_t mCurrentFrameRemaining;

};
