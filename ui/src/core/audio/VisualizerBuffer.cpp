
#include "core/audio/VisualizerBuffer.hpp"

#include <algorithm>

#include <QtGlobal>

VisualizerBuffer::VisualizerBuffer() :
    mBufferData(),
    mBufferSize(0),
    mIndex(0),
    mIgnoreCounter(0)
{
}

void VisualizerBuffer::clear() {
    std::fill_n(mBufferData.get(), mBufferSize * 2, (int16_t)0);
    mIndex = 0;
    mIgnoreCounter = 0;
}

void VisualizerBuffer::resize(size_t size) {

    if (mBufferSize != size) {
        mBufferSize = size;

        auto samples = size * 2;
        mBufferData.reset(new int16_t[samples]);

        // resize the buffer clears it
        clear();
    }
}

size_t VisualizerBuffer::size() const {
    return mBufferSize;
}


void VisualizerBuffer::read(size_t index, int16_t &outLeft, int16_t &outRight) {
    Q_ASSERT(index < mBufferSize);

    index = (index + mIndex) % mBufferSize;

    auto buf = mBufferData.get() + (index * 2);
    outLeft = *buf++;
    outRight = *buf;

}

void VisualizerBuffer::averageSample(float index, float bin, float &outLeft, float &outRight) {
    
    // determine the number of samples to average, with a minimum of 1 sample
    int samples = std::max(1, (int)((index + bin) - (int)index));

    size_t bufIndex = (mIndex + (size_t)index) % mBufferSize;
    auto buf = mBufferData.get() + (bufIndex * 2);

    int sumLeft = 0;
    int sumRight = 0;
    for (int i = 0; i < samples; ++i) {
        sumLeft += *buf++;
        sumRight += *buf++;
        if (++bufIndex == mBufferSize) {
            bufIndex = 0;
            buf = mBufferData.get();
        }
    }
    
    outLeft = (float)sumLeft / samples;
    outRight = (float)sumRight / samples;

}

void VisualizerBuffer::beginWrite(size_t amount) {

    if (amount > mBufferSize) {
        mIgnoreCounter = amount - mBufferSize;
    } else {
        mIgnoreCounter = 0;
    }
}

void VisualizerBuffer::write(int16_t buf[], size_t amount) {

    auto ignoring = std::min(mIgnoreCounter, amount);
    amount -= ignoring;
    mIgnoreCounter -= ignoring;

    if (amount) {
        // adjust buffer pointer skip whatever we ignored
        buf += ignoring * 2;

        auto dest = mBufferData.get() + (mIndex * 2);

        // this loop should only iterate two times max, if not then the amount is
        // greater than the buffer's capacity and the caller did not call
        // beginWrite first. Iterating more than twice is inefficient, since we
        // are overwriting what we just copied.
        for (;;) {
            auto const toWrite = std::min(mBufferSize - mIndex, amount);
            auto const toWriteSamples = toWrite * 2;
            std::copy_n(buf, toWriteSamples, dest);
            amount -= toWrite;
            mIndex += toWrite;

            if (mIndex == mBufferSize) {
                mIndex = 0;
                dest = mBufferData.get();
            } else {
                dest += toWriteSamples;
            }

            if (amount > 0) {
                buf += toWriteSamples;
            } else {
                break;
            }
        }

    }

}
