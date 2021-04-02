
#pragma once

#include "trackerboy/Synth.hpp"
#include "miniaudio.h"

#include <atomic>
#include <string>

constexpr auto DEVICE_FORMAT = ma_format_s16;
constexpr auto DEVICE_SAMPLERATE = 48000;
constexpr auto DEVICE_CHANNELS = 2;

class InteractiveDemo {


public:

    virtual ~InteractiveDemo();

    int exec();


protected:

    InteractiveDemo();

    // each demo program will implement these methods

    virtual int init() = 0;

    virtual bool processLine(std::string const& line) = 0;

    //
    // Called from the audio callback when synthesizing a new frame
    // do not use I/O, syscalls or block for any amount of time in this function!
    //
    virtual void runFrame() = 0;

    void lock();

    void unlock();

    trackerboy::Synth mSynth;

private:

    static void audioCallback(ma_device *device, void *out, const void *in, ma_uint32 frames);

    std::unique_ptr<ma_device> mDevice;
    std::atomic_flag mLock;



};
