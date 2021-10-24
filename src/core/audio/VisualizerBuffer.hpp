#pragma once

#include <cstddef>
#include <memory>


//
// Audio buffer for visualizers.
// 
// 
// The buffer is a rotating sample buffer, with newest data at the end of the buffer
// The index determines the starting and ending position of the buffer
// 
// 0 0 0 0 0
// ^
// 
// write A B C to the buffer (0 is now the oldest, C is the newest)
// 
// A B C 0 0
//       ^
// 
// write D E F to the buffer (B is now the oldest, F is the newest)
// 
// F B C D E
//   ^
// 
// When the index gets to the end of the buffer, it wraps (rotates) to the start
// The index points to the oldest sample in the buffer, the sample before it is the newest.
//
class VisualizerBuffer {

public:
    VisualizerBuffer();
    ~VisualizerBuffer() = default;

    void clear();

    void resize(size_t size);

    size_t size() const;

    void read(size_t index, float &outLeft, float &outRight);

    //
    // Computes the average left and right samples for the given index and bin
    // size. Bin size refers to the ratio of samples per pixel. ie, a bin size
    // of 2.5 means that 2.5 samples are represented by a single pixel.
    //
    void averageSample(float index, float bin, float &outLeft, float &outRight);

    //
    // Begin a write operation. If amount is greater than this buffer's
    // capacity, then some of the data written when calling write will
    // be ignored. ie, if the buffer capacity is 300 samples and we
    // begin a write for 500, the first 200 samples will be ignored. This
    // way, only the most recent samples are stored.
    //
    void beginWrite(size_t amount);

    void write(float buf[], size_t amount);


private:

    std::unique_ptr<float[]> mBufferData;
    size_t mBufferSize;

    size_t mIndex;

    size_t mIgnoreCounter;

};
