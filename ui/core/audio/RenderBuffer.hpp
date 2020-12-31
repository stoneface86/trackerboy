
#pragma once

#include "core/audio/RenderFrame.hpp"

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


    // for the audio callback thread:

    // reads the requested number of samples to the callback output buffer
    // the amount actually read is returned
    size_t read(int16_t *out, size_t samples);

    // queues an APU frame of audio data to the buffer
    void queueFrame(int16_t data[], size_t framesize);

private:

    std::unique_ptr<RenderFrame[]> mFrames;
    std::unique_ptr<int16_t[]> mSampleBuffer;

    unsigned mBuffersize;
    size_t mSamplesPerFrame;

    int mReadIndex;
    int mWriteIndex;
    unsigned mFramesAvailable;

    int16_t *mCurrentFrameData;
    size_t mCurrentFrameRemaining;

};
