
#include "SynthWorker.hpp"



namespace instedit {


SynthWorker::SynthWorker(QObject *parent) :
    loop(false),
    shouldStop(false),
    synth(44100),
    pb(44100),
    runtime(nullptr),
    QThread(parent)
{
    auto &mixer = synth.getMixer();
    mixer.setTerminalEnable(trackerboy::Mixer::term_both, true);
    mixer.setEnable(trackerboy::Mixer::all_on);

    buf.reset(new float[pb.framesize() * 2]);
}

void SynthWorker::run() {

    if (runtime == nullptr) { // eh
        return;
    }

    shouldStop = false;
    runtime->reset();
    
    // begin playback
    //pb.start();

    // frame buffer pointers
    auto frame = buf.get();
    size_t framesize = pb.framesize();

    bool stoppedEarly = false;
    // loop
    for (;;) {
        if (runtime->isFinished()) {
            if (loop) {
                runtime->reset();
            } else {
                break;
            }
        }
        // sleep for a bit if we cannot write any frames to the buffer
        // TODO: add a method to Playback to get the optimal sleep time (using 100 ms)
        mutex.lock();
        while (!pb.canWrite()) {
            writeAvailable.wait(&mutex, 100);
        }
        stoppedEarly = shouldStop;
        mutex.unlock();

        // quit the loop if stop() was called
        if (stoppedEarly) {
            break;
        }

        // execute 1 frame of the program
        runtime->step(synth);
        // synthesize the frame
        synth.fill(frame, framesize);
        // write it to the playback buffer
        pb.writeFrame(frame);
        

    }

    // if we stopped early, do not wait to playback whatever is in the buffer
    pb.stop(!stoppedEarly);
}

void SynthWorker::setFrequency(uint16_t frequency) {
    auto &cf = synth.getChannels();
    cf.ch1.setFrequency(frequency);
    cf.ch2.setFrequency(frequency);
    cf.ch3.setFrequency(frequency);
}

void SynthWorker::setRuntime(trackerboy::InstrumentRuntime *_runtime) {
    runtime = _runtime;
}

void SynthWorker::stop() {
    mutex.lock();
    shouldStop = true;
    mutex.unlock();
    // wake up this thread
    writeAvailable.notify_all();
}

void SynthWorker::setLoop(bool _loop) {
    loop = _loop;
}



}