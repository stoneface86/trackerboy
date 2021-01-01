
#include "core/audio/RenderBuffer.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>

RenderBuffer::RenderBuffer() :
    mFrames(),
    mSampleBuffer(),
    mBuffersize(0),
    mSamplesPerFrame(0),
    mReadIndex(0),
    mWriteIndex(0),
    mFramesAvailable(0),
    mCurrentFrameData(nullptr),
    mCurrentFrameRemaining(0)
{
}

RenderBuffer::~RenderBuffer() {

}

bool RenderBuffer::isEmpty() const {
    return mCurrentFrameRemaining == 0 && mFramesAvailable == 0;
}

unsigned RenderBuffer::size() const {
    return mBuffersize;
}

void RenderBuffer::reset() {
    mReadIndex = 0;
    mWriteIndex = 0;
    mFramesAvailable = 0;
    mCurrentFrameData = nullptr;
    mCurrentFrameRemaining = 0;
}

void RenderBuffer::setSize(unsigned frameCount, size_t samplesPerFrame) {
    assert(frameCount != 0);
    assert(samplesPerFrame != 0);

    bool resize = false;

    if (mBuffersize != frameCount) {
        resize = true;
        mBuffersize = frameCount;
    }
    if (mSamplesPerFrame != samplesPerFrame) {
        resize = true;
        mSamplesPerFrame = samplesPerFrame;
    }

    if (resize) {
        auto const maxSamplesPerFrame = (samplesPerFrame + 1) * 2;

        mFrames.reset(new RenderFrame[frameCount]);
        mSampleBuffer.reset(new int16_t[maxSamplesPerFrame * frameCount]);

        // setup data pointers for each frame
        auto frames = mFrames.get();
        auto buffer = mSampleBuffer.get();
        
        for (unsigned i = 0; i != frameCount; ++i) {
            frames->data = buffer;
            frames++;
            buffer += maxSamplesPerFrame;
        }
        reset();
    }

}

unsigned RenderBuffer::framesToQueue() {
    auto toQueue = mBuffersize - mFramesAvailable;
    return mCurrentFrameRemaining == 0 ? toQueue : toQueue - 1;
}

unsigned RenderBuffer::framesInQueue() {
    return mFramesAvailable;
}

size_t RenderBuffer::read(int16_t *out, size_t samples) {
    
    size_t samplesRead = 0;
    while (samples) {
        
        if (mCurrentFrameRemaining == 0) {
            if (mFramesAvailable == 0) {
                // nothing is queued, stop
                break;
            }

            if (mReadIndex == mBuffersize) {
                // wrap back to the start
                mReadIndex = 0;
            }

            auto frame = mFrames.get() + mReadIndex;
            mCurrentFrameRemaining = frame->nsamples;
            mCurrentFrameData = frame->data;
            ++mReadIndex;
            --mFramesAvailable;
        }


        size_t toCopy = std::min(samples, mCurrentFrameRemaining);
        size_t toCopy2 = toCopy * 2;
        std::copy_n(mCurrentFrameData, toCopy2, out);
        mCurrentFrameData += toCopy2;
        out += toCopy2;
        mCurrentFrameRemaining -= toCopy;

        samplesRead += toCopy;
        samples -= toCopy;
    }
    
    return samplesRead;
}

void RenderBuffer::queueFrame(int16_t data[], size_t framesize) {
    assert(mFramesAvailable != mBuffersize);

    if (mWriteIndex == mBuffersize) {
        mWriteIndex = 0; // wrap around
    }

    auto frame = mFrames.get() + mWriteIndex;
    // update sample size
    frame->nsamples = framesize;
    // copy sample data
    std::copy_n(data, framesize * 2, frame->data);

    ++mWriteIndex;
    ++mFramesAvailable;
    
}

