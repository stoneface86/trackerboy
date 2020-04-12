
#pragma once

#include <stdexcept>
#include <memory>
#include <vector>

#include "portaudio.h"
#include "pa_ringbuffer.h"

namespace audio {


//
// Exception class for a portaudio error. The portaudio error code
// that caused this exception can be accessed via the getError() method
//
class PaException : public std::runtime_error {
    PaError err;
public:
    PaException(PaError err);

    PaError getError();
};

//
// Class for an audio playback queue. Samples to be played out are stored in
// the queue by calling write or writeAll. The samples will be played out
// to the default device when the stream is started.
//
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
    // Minimum size of the playback queue, in milleseconds.
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
    // Change the minimum buffer size of the playback queue. The given size, in
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
    size_t write(float buf[], size_t nsamples);

    //
    // Write the entire sample buffer to the playback queue. This method will
    // block until the entire buffer is written to the queue.
    //
    void writeAll(float buf[], size_t nsamples);

private:

    PaStream *mStream;
    PaUtilRingBuffer mQueue;

    std::vector<float> mQueueData;

    float mSamplingRate;
    unsigned mBufferSize; // size in milleseconds of the buffer
    unsigned mWaitTime; // time in milleseconds to sleep when buffer is full
    

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

