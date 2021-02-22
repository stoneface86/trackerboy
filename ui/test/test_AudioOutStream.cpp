
#include "core/audio/AudioOutStream.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>

using namespace std::chrono;

constexpr unsigned SAMPLERATE = 48000;
constexpr auto SAMPLE_FORMAT = ma_format_s16;
constexpr auto SAMPLE_CHANNELS = 2;

constexpr unsigned TEST_DURATION = 3;
constexpr unsigned ALLOWED_UNDERRUNS = 2;
constexpr unsigned EXPECTED_SAMPLES_ELAPSED = SAMPLERATE * TEST_DURATION;

static auto MSG_INFO =  "[=== INFO ===] ";
static auto MSG_FAIL =  "[=== FAIL ===] ";
static auto MSG_OK =    "[===  OK  ===] ";
static auto MSG_SEPARATOR = "--------------------------------------------------------------------------------";

static ma_waveform SINE_GENERATOR;

void fillBuffer(AudioRingbuffer::Writer &buffer) {
    auto available = buffer.availableWrite();
    auto toWrite = available;
    auto dataPtr = buffer.acquireWrite(toWrite);
    ma_waveform_read_pcm_frames(&SINE_GENERATOR, dataPtr, toWrite);
    buffer.commitWrite(dataPtr, toWrite);
    if (available != toWrite) {
        fillBuffer(buffer);
    }

}


bool test(AudioOutStream &stream, ma_device *device, unsigned latency = 0) {
    stream.resetUnderruns();

    // setup the device for testing
    auto config = ma_device_config_init(ma_device_type_playback);
    // always 16-bit stereo format
    config.playback.format = SAMPLE_FORMAT;
    config.playback.channels = SAMPLE_CHANNELS;
    config.periodSizeInMilliseconds = latency;
    config.sampleRate = SAMPLERATE;
    config.dataCallback = AudioOutStream::callback;
    config.pUserData = &stream;

    // initialize device with settings
    auto err = ma_device_init(nullptr, &config, device);
    assert(err == MA_SUCCESS);

    unsigned actualLatency = device->playback.internalPeriodSizeInFrames * 1000 / SAMPLERATE;

    std::cout << MSG_INFO
        << "Requested latency: " << latency << " ms, "
        << "Actual latency: " << actualLatency << " ms "
        << "(" << device->playback.internalPeriodSizeInFrames << " samples)" << std::endl;

    stream.setDevice(device);

    auto buffer = stream.buffer();
    unsigned syncCounter = 0;
    long long syncSum = 0;


    fillBuffer(buffer);

    std::cout << MSG_INFO << "Beginning playback..." << std::endl;

    stream.start();
    stream.setIdle(false);
    auto stopTime = steady_clock::now() + seconds(TEST_DURATION) - milliseconds(actualLatency);
    for (;;) {
        // sync with the stream, blocks until a full period has been played out
        stream.sync();
        syncCounter++; 
        syncSum += stream.syncTime();

        if (steady_clock::now() >= stopTime) {
            break;
        }

        fillBuffer(buffer);
    }
    stream.setIdle(true);
    stream.stop();

    auto avgSyncTime = (syncSum / syncCounter) * 1e-6;
    std::cout << MSG_INFO << "Average sync time: " << avgSyncTime << " ms" 
        << " (" << syncCounter << " syncs)"
        << std::endl;

    // display results
    auto underruns = stream.underruns();
    auto elapsed = stream.elapsed();
    bool allpassed = true;

    // check that we had less than 2 underruns
    if (underruns >= ALLOWED_UNDERRUNS) {
        std::cout << MSG_FAIL;
        allpassed = false;
    } else {
        std::cout << MSG_OK;
    }
    std::cout << "Underruns: " << underruns << std::endl;

    // check that we played at least 3 seconds of audio (tests synchronization)
    if (elapsed < EXPECTED_SAMPLES_ELAPSED) {
        std::cout << MSG_FAIL;
        allpassed = false;
    } else {
        std::cout << MSG_OK;
    }
    std::cout << "Samples played: " << elapsed << std::endl;



    ma_device_uninit(device);

    return allpassed;

}



int main() {

    auto waveConfig = ma_waveform_config_init(
        SAMPLE_FORMAT, 
        SAMPLE_CHANNELS,
        SAMPLERATE,
        ma_waveform_type_sine,
        0.6,
        440.0
    );

    ma_waveform_init(&waveConfig, &SINE_GENERATOR);

    AudioOutStream stream;

    std::unique_ptr<ma_device> device(new ma_device);

    std::cout << "Playing " << TEST_DURATION << " seconds of a sine wave @ 440 Hz" << std::endl;
    std::cout << MSG_SEPARATOR << std::endl;

    std::array TESTS = {
        1,
        5,
        10,
        30,
        50,
        100
    };

    unsigned passes = 0;
    for (auto latency : TESTS) {
        if (test(stream, device.get(), latency)) {
            passes++;
        }
        std::cout << MSG_SEPARATOR << std::endl;
    }
    std::cout << passes << "/" << TESTS.size() << " tests passed" << std::endl;

    return 0;
}
