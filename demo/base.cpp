
#include <iostream>
#include <cstdlib>
#include <mutex>

#include "portaudio.h"
#include "gbsynth.hpp"
#include "base.h"

#define checkerr_pa(err) if (err != paNoError) quit_pa(err)

#define SAMPLING_RATE 44100

// callback user data
struct CbData {
    std::mutex mutex;
    gbsynth::Synth synth;

    CbData(float samplingRate) :
        synth(samplingRate)
    {
    }
};


//
// Portaudio error occured, exit
//
static void quit_pa(PaError err) {
    std::cerr << "portaudio error: " << Pa_GetErrorText(err) << std::endl;
    Pa_Terminate();
    exit(EXIT_FAILURE);
}


static int synthCallback(
    const void* inputBuffer,
    void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData
) {
    (void)inputBuffer; // not used
    // non-interleaved, outputBuffer is an array of float buffers
    float *leftBuf = ((float**)outputBuffer)[0];
    float *rightBuf = ((float**)outputBuffer)[1];
    auto data = (CbData*)userData;

    // synthesize
    data->mutex.lock();
    data->synth.fill(leftBuf, rightBuf, framesPerBuffer);
    data->mutex.unlock();
    return 0;
}


int main(int argc, const char *argv[]) {

    // setup the synthesizer
    CbData data(SAMPLING_RATE);

    int demoerr = demo::init(data.synth);
    if (demoerr) {
        std::cerr << "error during demo initialization: " << demoerr << std::endl;
        return demoerr;
    }

    // init portaudio
    PaError err = Pa_Initialize();
    checkerr_pa(err);

    // setup the stream
    PaStream *stream;
    err = Pa_OpenDefaultStream(
        &stream,
        0,                              // no input channels
        2,                              // stereo output (2 output channels)
        paFloat32 | paNonInterleaved,   // 32 bit float output, separate left/right buffers
        SAMPLING_RATE,                  // 44.1 KHz sampling rate
        paFramesPerBufferUnspecified,   // frames per buffer (can also use PaFramesPerBufferUnspecified)
        synthCallback,                  // callback function
        &data                           // userdata is our synth object
    );
    checkerr_pa(err);

    // setup the synthesizer for the first demo run
    int cont = demo::RUN_START;
    long runtime;
    demoerr = demo::run(cont, data.synth, runtime);

    err = Pa_StartStream(stream);
    checkerr_pa(err);

    // run until we get an error or runtime == 0 (no more demos)
    while (demoerr == demo::NOERROR && runtime) {
        Pa_Sleep(runtime);  // delay, let the callback output sound for a bit
        // get the next demo
        data.mutex.lock();
        demoerr = demo::run(++cont, data.synth, runtime);
        data.mutex.unlock();
    }

    err = Pa_StopStream(stream);
    checkerr_pa(err);

    if (demoerr != demo::NOERROR) {
        std::cerr << "error during demo run: " << demoerr << std::endl;
    }

    // cleanup
    Pa_Terminate();
    return demoerr;
}