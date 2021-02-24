
#include "core/audio/RenderBuffer.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>

RenderBuffer::RenderBuffer() :
    mSamples(),
    mFrames(),
    mBuffersize(0),
    mSamplesPerFrame(0),
    mFramesAvailable(0),
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
    mFramesAvailable = 0;
    mCurrentFrameRemaining = 0;
}

RenderFrame const& RenderBuffer::popFrame() {
    mNewFrame = false;
    return mCurrentFrame;
}

bool RenderBuffer::hasNewFrame() {
    return mNewFrame;
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
    
    auto frameReader = mFrames.reader();
    auto sampleReader = mSamples.reader();

    size_t samplesRead = 0;
    while (samples) {
        
        if (mCurrentFrameRemaining == 0) {
            if (mFramesAvailable == 0) {
                // nothing is queued, stop
                break;
            }

            // transfer the next frame to the return buffer

            size_t readCount = 1;
            auto frame = frameReader.acquireRead(readCount);
            // there should always be 1 frame to read at this point
            mCurrentFrameRemaining = frame->nsamples;
            mCurrentFrame = *frame;
            mNewFrame = true;

            frameReader.commitRead(frame, readCount);

            
            --mFramesAvailable;
        }



        size_t toCopy = std::min(samples, mCurrentFrameRemaining);
        sampleReader.fullRead(out, toCopy);

        out += toCopy * 2;
        mCurrentFrameRemaining -= toCopy;

        samplesRead += toCopy;
        samples -= toCopy;
    }
    
    return samplesRead;
}

void RenderBuffer::queueFrame(int16_t data[], RenderFrame const& frame) {
    assert(mFramesAvailable != mBuffersize);

    mFrames.writer().fullWrite(&frame, 1);
    mSamples.writer().fullWrite(data, frame.nsamples);


    ++mFramesAvailable;
    
}

