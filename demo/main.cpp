
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif


#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "demo.hpp"
#include "trackerboy/synth/Synth.hpp"
#include "portaudio.h"



#define SAMPLING_RATE 44100



// callback user data
struct CbData {
    std::mutex mutex;
    trackerboy::Synth *synth;

    CbData() :
        synth(nullptr)
    {
    }
};


static void runDemo(CbData &cbdata, Demo *demo) {
    assert(demo != nullptr); // should never be null

    std::cout << std::left;
    std::cout << std::setfill('=') << std::setw(80) << ("=== DEMO: " + demo->getName() + " ") << std::endl;

    // initialize the synth
    // each demo starts with a new synth object
    trackerboy::Synth synth(SAMPLING_RATE);
    cbdata.mutex.lock();
    cbdata.synth = &synth; // update the callback's reference
    demo->init(synth);     // initialize demo
    cbdata.mutex.unlock();

    unsigned counter = 0;
    long runtime;
    for (;;) {
        // setup the synthesizer for the current run
        cbdata.mutex.lock();
        runtime = demo->setupNextRun(synth, counter++);
        cbdata.mutex.unlock();
        if (runtime == 0) {
            // no more runs, stop
            break;
        } else {
            // sleep to let the callback function output sound
            Pa_Sleep(runtime);
        }
    }

    // set the callback synth to null (callback will output silence in the meantime)
    cbdata.mutex.lock();
    cbdata.synth = nullptr;
    cbdata.mutex.unlock();

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
    int16_t *buf = static_cast<int16_t*>(outputBuffer);
    auto data = (CbData*)userData;

    // synthesize
    data->mutex.lock();
    if (data->synth == nullptr) {
        std::fill_n(buf, framesPerBuffer, 0);
    } else {
        data->synth->fill(buf, framesPerBuffer);
    }
    data->mutex.unlock();
    return 0;
}


int main(int argc, const char *argv[]) {

    // memory leak checking
#ifdef _MSC_VER
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

    // map of demo name -> demo object
    std::map<std::string, std::unique_ptr<Demo>> demoTable;
    #define demoTableSet(name, ptr) demoTable[name] = std::unique_ptr<Demo>(ptr)
    demoTableSet("duty", new DutyDemo());
    demoTableSet("sweep", new SweepDemo());
    demoTableSet("waveTriangle", new WaveDemo(WAVEDATA_TRIANGLE, "Triangle"));
    demoTableSet("waveSawtooth", new WaveDemo(WAVEDATA_SAWTOOTH, "Sawtooth"));
    demoTableSet("waveSine", new WaveDemo(WAVEDATA_SINE, "Sine"));
    demoTableSet("waveVol", new WaveVolDemo());
    #undef demoTableSet

    // get our list of demos to run
    // if no arguments are given, run all demos
    // otherwise only run the given arguments
    std::vector<Demo*> demoVec;
    if (argc > 1) {
        // only run the given demo names
        for (int i = 1; i != argc; ++i) {
            std::string name = argv[i];
            auto iter = demoTable.find(name);
            if (iter == demoTable.end()) {
                std::cerr << "invalid demo '" << name << "'" << std::endl;
                return 1;
            } else {
                demoVec.push_back(iter->second.get());
            }
        }
    } else {
        for (auto iter = demoTable.begin(); iter != demoTable.end(); ++iter) {
            demoVec.push_back(iter->second.get());
        }
    }

    CbData data;

    // this loop (not really) is for error handling
    // (portaudio examples like to use gotos so I'm doing something different)
    PaError err;
    do {

        // init portaudio
        err = Pa_Initialize();
        if (err != paNoError) break;

        // setup the stream
        PaStream *stream;
        err = Pa_OpenDefaultStream(
            &stream,
            0,                              // no input channels
            2,                              // stereo output (2 output channels)
            paInt16,                        // 16-bit integer samples
            SAMPLING_RATE,                  // 44.1 KHz sampling rate
            paFramesPerBufferUnspecified,   // frames per buffer (can also use PaFramesPerBufferUnspecified)
            synthCallback,                  // callback function
            &data                           // userdata is our synth object
        );
        if (err != paNoError) break;



        err = Pa_StartStream(stream);
        if (err != paNoError) break;

        for (auto it = demoVec.begin(); it != demoVec.end(); ++it) {
            runDemo(data, *it);
        }

        err = Pa_StopStream(stream);
        if (err != paNoError) break;

        // no errors, exit program here

        // cleanup
        Pa_Terminate();
        return EXIT_SUCCESS;
        
    } while (0);

    // error occured since we left the loop
    std::cerr << "portaudio error: " << Pa_GetErrorText(err) << std::endl;
    // cleanup
    Pa_Terminate();
    return err;
}
