
#include "core/audio/AudioOutStream.hpp"

#include <QDeadlineTimer>

// if defined sync() will poll the mSync variable instead of waiting on it
//#define SYNC_POLLING


// implementation notes
// the audio callback uses a condition variable for synchronization. The callback
// thread signals the condition variable when a period of samples has been played out.
// The callback is called in real-time so using a condition variable is not ideal as
// signaling most likely results in a system call and its execution is not time-bounded.
// This may result in missed deadlines or gaps in audio playback if the call takes too
// long to execute (depends on the OS, system load, etc).
//
// An alternative solution is to use polling, which uses more CPU time and results
// in less accurate synchronization (which may also result in underruns).
//
// Since both solutions may result in underruns/gaps, the condition variable approach
// will be used since it requires less CPU time and is more accurate. If any glitches
// occur the user can increase the latency (period size and/or count).

AudioOutStream::AudioOutStream() :
    mDevice(nullptr),
    mSyncTimeout(1),
    mSync(false),
    mIdling(false),
    mUnderruns(0),
    mSamplesElapsed(0),
    mSyncCondition(),
    mBuffer(),
    mSyncCounter(0),
    mSyncPeriod(0),
    mLastSyncTime(),
    mLatency(0)
{
}

AudioRingbuffer::Writer AudioOutStream::buffer() {
    return mBuffer.writer();
}

size_t AudioOutStream::bufferSize() {
    return mBuffer.size();
}

unsigned AudioOutStream::underruns() {
    return mUnderruns.load();
}

unsigned AudioOutStream::elapsed() {
    return mSamplesElapsed.load();
}

long long AudioOutStream::syncTime() {
    return mLatency.count();
}

bool AudioOutStream::finished() {
    return mIdling && mBuffer.reader().availableRead() == 0;
}

void AudioOutStream::interrupt() {
    QMutexLocker locker(&mMutex);
    mSyncCondition.wakeOne();
}

void AudioOutStream::resetUnderruns() {
    mUnderruns = 0;
}

void AudioOutStream::setDevice(ma_device *device) {
    // if a stream is running we'll need to hijack it to the new device
    if (mDevice != nullptr && ma_device_is_started(mDevice)) {
        auto idling = mIdling.load();
        mIdling = true;

        // stop the old device
        ma_device_stop(mDevice);

        initCallback(device);

        // start the new device
        ma_device_start(device);

        mIdling = idling;
    } else {
        initCallback(device);
    }


    mDevice = device;

    

}

void AudioOutStream::setIdle(bool idle) {
    mIdling = idle;
}

ma_result AudioOutStream::start() {
    auto result = ma_device_start(mDevice);
    if (result == MA_SUCCESS) {
        mSync = false;
        mSamplesElapsed = 0;
        mLastSyncTime = Clock::now();
    } else {
        mBuffer.reset();
    }
    return result;
}

void AudioOutStream::stop() {
    auto result = ma_device_stop(mDevice);
    if (result == MA_SUCCESS) {
        mBuffer.reset();
    }
}

void AudioOutStream::sync() {
    assert(ma_device_is_started(mDevice));

    mMutex.lock();

    while (!mSync) {
        // since the callback does not lock the mutex, a timeout is used to prevent
        // lost wakeups
        mSyncCondition.wait(&mMutex, QDeadlineTimer(mSyncTimeout, Qt::PreciseTimer));
    }

    auto now = Clock::now();
    mLatency = now - mLastSyncTime;
    mLastSyncTime = now;

    mSync = false;
    mMutex.unlock();
}

void AudioOutStream::callback(
    ma_device *device,
    void *out,
    const void *in,
    ma_uint32 frames
) {
    (void)in;
    static_cast<AudioOutStream*>(device->pUserData)->handleCallback(
        static_cast<int16_t*>(out), 
        frames
    );
}

void AudioOutStream::handleCallback(int16_t *out, size_t frames) {

    // read from the buffer to the device
    auto read = mBuffer.reader().fullRead(out, frames);
    if (!mIdling && read != frames) {
        // not enough in the buffer for this call = underrun!
        ++mUnderruns;
    }
    mSyncCounter += frames;
    mSamplesElapsed += (unsigned)frames;

    // audio sync
    if (mSyncCounter >= mSyncPeriod) {
        // sync audio when we have written at least mSyncPeriod samples
        mSyncCounter %= mSyncPeriod;
        mSync = true;
        
        // might be risky to do this here but it beats polling
        mSyncCondition.wakeOne();
        
    }
}

void AudioOutStream::initCallback(ma_device *device) {
    // the sync period is the device's internal period size
    mSyncPeriod = device->playback.internalPeriodSizeInFrames;
    mSyncCounter = 0;

    // resize/init buffer
    mBuffer.init(mSyncPeriod * device->playback.internalPeriods);

    mSyncTimeout = (((unsigned)mSyncPeriod * 1000) - 1) / device->sampleRate + 1;

}
