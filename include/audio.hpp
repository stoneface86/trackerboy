
#pragma once

#include <stdexcept>
#include <memory>
#include <vector>

#include "portaudio.h"
#include "pa_ringbuffer.h"

namespace audio {


class PaException : public std::runtime_error {
    PaError err;
public:
    PaException(PaError err);

    PaError getError();
};


class PlaybackQueue {

public:

    // maximum buffer size of 500 milleseconds
    static constexpr unsigned MAX_BUFFER_SIZE = 500;
    static constexpr unsigned MIN_BUFFER_SIZE = 1;
    static constexpr unsigned DEFAULT_BUFFER_SIZE = 40;


    PlaybackQueue(float samplingRate, unsigned bufferSize = DEFAULT_BUFFER_SIZE);
    ~PlaybackQueue();

    //
    // Size of the playback buffer, by number of samples.
    //
    size_t bufferSampleSize();

    //
    // Size of the playback queue, in milleseconds.
    //
    unsigned bufferSize();

    //
    // Check if a write can be made to the queue for the given number
    // of samples.
    //
    bool canWrite(size_t nsamples);

    //
    // Empty the playback queue
    //
    void flush();

    //
    // Change the buffer size of the playback queue. The given size, in
    // milleseconds, should be in the range of MIN_BUFFER_SIZE and
    // MAX_BUFFER_SIZE
    //
    void setBufferSize(unsigned bufferSize);

    //
    // Change the sampling rate for the playback output. The stream must
    // be stopped before calling this method, as a new one will have to be
    // opened. The queue is also flushed.
    //
    void setSamplingRate(float samplingRate);

    //
    // Begin audio playback. Audio data written to the queue is then
    // played out to the speakers. Silence is played out whenever the
    // queue is empty.
    //
    void start();

    //
    // Force stop of the playback stream. If wait is true, then this method
    // will block until all samples in the playback queue are played out.
    // Otherwise, the stream terminates immediately. The queue is also flushed.
    //
    void stop(bool wait);

    //
    // Write the given sample buffer to the playback queue. The stream is then
    // started if the stream was stopped (queue was empty or stop() was called).
    //
    size_t write(int16_t buf[], size_t nsamples);

    //
    // Write the entire sample buffer to the playback queue. If the queue is does
    // not have enough spaces available then this method will wait until there
    // is room.
    //
    void writeAll(int16_t buf[], size_t nsamples);

private:

    PaStream *mStream;
    PaUtilRingBuffer mQueue;

    std::vector<int16_t> mQueueData;

    float mSamplingRate;
    unsigned mBufferSize; // size in milleseconds of the buffer
    size_t mSlack; // unusable slots in the queue since the size of the queue must be a power of 2
    

    friend PaStreamCallback playbackCallback;

    //
    // Check if the given bufferSize is valid, throw invalid_argument otherwise
    // A valid buffer size is withing the range of MIN_BUFFER_SIZE and MAX_BUFFER_SIZE, inclusive
    //
    void checkBufferSize(unsigned bufferSize);

    //
    // Check if the given sampling rate is valid, throw invalid_argument otherwise
    // A valid sampling rate is nonzero and positive.
    //
    void checkSamplingRate(float samplingRate);

    //
    // Opens the PaStream stored in mStream
    //
    void openStream();

    void resizeQueue();
};



}

