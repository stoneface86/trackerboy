
#include "core/audio/RenderBuffer.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>

RenderBuffer::RenderBuffer() :
    mSamples(),
    mFrames(),
    mReturnFrames(),
    mBuffersize(0),
    mSamplesPerFrame(0),
    mFramesAvailable(0),
    mCurrentFrameRemaining(0)
{
    // 4 frame buffer so that the GUI thread won't miss any
    mReturnFrames.init(4);
}

RenderBuffer::~RenderBuffer() {

}

Ringbuffer<RenderFrame>& RenderBuffer::returnFrames() {
    return mReturnFrames;
}

bool RenderBuffer::isEmpty() const {
    return mCurrentFrameRemaining == 0 && mFramesAvailable == 0;
}

unsigned RenderBuffer::size() const {
    return mBuffersize;
}

void RenderBuffer::reset() {
    mFramesAvailable = 0;
    mCurrentFrameRemaining = 0;
}

void RenderBuffer::setSize(unsigned frameCount, size_t samplesPerFrame) {
    assert(frameCount != 0);
    assert(samplesPerFrame != 0);

    bool resize = false;

    if (mBuffersize != frameCount) {
        resize = true;
        mBuffersize = frameCount;
        mFrames.init(frameCount);
    }
    if (mSamplesPerFrame != samplesPerFrame) {
        resize = true;
        mSamplesPerFrame = samplesPerFrame;
        mSamples.init((samplesPerFrame + 1) * frameCount);
    }

    if (resize) {
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

            // transfer the next frame to the return buffer

            size_t readCount = 1;
            auto frame = mFrames.acquireRead(readCount);
            // there should always be 1 frame to read at this point
            mCurrentFrameRemaining = frame->nsamples;

            size_t writeCount = 1;
            auto returnFrame = mReturnFrames.acquireWrite(writeCount);
            // if writeCount is 0 then the GUI thread is out of sync
            if (writeCount) {
                *returnFrame = *frame;
            }
            mReturnFrames.commitWrite(returnFrame, writeCount);
            mFrames.commitRead(frame, readCount);

            
            --mFramesAvailable;
        }



        size_t toCopy = std::min(samples, mCurrentFrameRemaining);
        mSamples.fullRead(out, toCopy);

        out += toCopy * 2;
        mCurrentFrameRemaining -= toCopy;

        samplesRead += toCopy;
        samples -= toCopy;
    }
    
    return samplesRead;
}

void RenderBuffer::queueFrame(int16_t data[], RenderFrame const& frame) {
    assert(mFramesAvailable != mBuffersize);

    mFrames.fullWrite(&frame, 1);
    mSamples.fullWrite(data, frame.nsamples);


    ++mFramesAvailable;
    
}

