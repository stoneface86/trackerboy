
#include "SynthWorker.hpp"

static constexpr float SAMPLING_RATE = 44100.0f;
static constexpr float FRAMERATE = 59.7f;
static constexpr size_t FRAMESIZE = (SAMPLING_RATE / FRAMERATE);


namespace instedit {

SynthWorker::SynthWorker(QObject *parent) :
    loop(false),
    shouldStop(false),
    synth(SAMPLING_RATE),
    pb(SAMPLING_RATE, 40),
    buf(new float[FRAMESIZE * 2]),
    runtime(nullptr),
    QThread(parent)
{
    auto &mixer = synth.getMixer();
    mixer.setTerminalEnable(trackerboy::Mixer::term_both, true);
    mixer.setEnable(trackerboy::Mixer::all_on);
    //setPriority(QThread::TimeCriticalPriority);
}

void SynthWorker::run() {

    if (runtime == nullptr) { // eh
        return;
    }

    shouldStop = false;
    runtime->reset();
    
    // begin playback
    pb.start();

    // frame buffer pointers
    auto frame = buf.get();

    bool stoppedEarly = false;
    // loop
    do {
        if (runtime->isFinished()) {
            if (loop) {
                runtime->reset();
            } else {
                break;
            }
        }
        
        synthMutex.lock();
        // execute 1 frame of the program
        runtime->step(synth);
        // synthesize the frame
        synth.fill(frame, FRAMESIZE);

        synthMutex.unlock();

        // write it to the playback buffer
        float *fp = frame;
        size_t written = 0;
        size_t toWrite = FRAMESIZE;
        while ((written = pb.write(fp, toWrite)) != toWrite) {
            toWrite -= written;
            fp += written * 2;
            mutex.lock();
            //while (!pb.canWrite(toWrite)) {
                writeAvailable.wait(&mutex, 10);
            //}
            stoppedEarly = shouldStop;
            mutex.unlock();

            if (stoppedEarly) {
                break;
            }

        }
        

    } while (!stoppedEarly);

    // if we stopped early, do not wait to playback whatever is in the buffer
    pb.stop(!stoppedEarly);
}

void SynthWorker::setFrequency(uint16_t frequency) {
    synthMutex.lock();
    auto &cf = synth.getChannels();
    cf.ch1.setFrequency(frequency);
    cf.ch2.setFrequency(frequency);
    cf.ch3.setFrequency(frequency);
    synthMutex.unlock();
    //pb.flush();
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