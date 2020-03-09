
#include "SynthWorker.hpp"

static constexpr float SAMPLING_RATE = 44100.0f;
static constexpr float FRAMERATE = 59.7f;
static constexpr size_t FRAMESIZE = (SAMPLING_RATE / FRAMERATE);


namespace instedit {

static const uint16_t NOTE_TABLE[] = {
    0x02C, 0x09D, 0x107, 0x16B, 0x1CA, 0x223, 0x277, 0x2C7, 0x312, 0x358, 0x39B, 0x3DA, //   2
    0x416, 0x44E, 0x483, 0x4B5, 0x4E5, 0x511, 0x53C, 0x563, 0x589, 0x5AC, 0x5CE, 0x5ED, //   3
    0x60B, 0x627, 0x642, 0x65B, 0x672, 0x689, 0x69E, 0x6B2, 0x6C4, 0x6D6, 0x6E7, 0x6F7, //   4
    0x706, 0x714, 0x721, 0x72D, 0x739, 0x744, 0x74F, 0x759, 0x762, 0x76B, 0x773, 0x77B, //   5
    0x783, 0x78A, 0x790, 0x797, 0x79D, 0x7A2, 0x7A7, 0x7AC, 0x7B1, 0x7B6, 0x7BA, 0x7BE, //   6
    0x7C1, 0x7C5, 0x7C8, 0x7CB, 0x7CE, 0x7D1, 0x7D4, 0x7D6, 0x7D9, 0x7DB, 0x7DD, 0x7DF, //   7
    0x7E1, 0x7E2, 0x7E4, 0x7E6, 0x7E7, 0x7E9, 0x7EA, 0x7EB, 0x7EC, 0x7ED, 0x7EE, 0x7EF  //   8
};

SynthWorker::SynthWorker(QObject *parent) :
    pb(SAMPLING_RATE),
    synth(SAMPLING_RATE),
    timer(new QTimer(this)),
    buf(new int16_t[FRAMESIZE * 2]),
    runtime(nullptr),
    playing(false),
    frameOut(false),
    frameOffset(0),
    QObject(parent)
{

    connect(timer, &QTimer::timeout, this, &SynthWorker::onTimeout);

    auto &mixer = synth.mixer();
    mixer.setTerminalEnable(trackerboy::Gbs::TERM_BOTH, true);
    mixer.setEnable(trackerboy::Gbs::OUT_ALL);
    //pb.start();
}

SynthWorker::~SynthWorker() {
    //pb.stop(false);
}

void SynthWorker::play(trackerboy::Note note) {
    synthMutex.lock();
    runtime->reset();
    auto &cf = synth.getChannels();
    uint16_t freq = NOTE_TABLE[static_cast<int>(note)];
    cf.ch1.setFrequency(freq);
    cf.ch2.setFrequency(freq);
    cf.ch3.setFrequency(freq);
    synthMutex.unlock();

    if (!playing) {
        // start the timer with interval 1 half the playback buffer size
        pb.start();
        //timer->setInterval(pb.bufferSize() / 4);
        timer->setInterval(0);
        timer->start();
        playing = true;
    }
}

void SynthWorker::stop() {
    if (playing) {
        playing = false;
    }
}

void SynthWorker::setRuntime(trackerboy::InstrumentRuntime *_runtime) {
    runtime = _runtime;
}


void SynthWorker::onTimeout() {
    auto frame = buf.get();
    if (frameOut) {
        // we are outputting a frame
        size_t toWrite = FRAMESIZE - frameOffset;
        size_t nwritten = pb.write(frame + (frameOffset * 2), toWrite);
        if (nwritten == toWrite) {
            // frame complete, make the next one asap
            timer->setInterval(0);
            frameOut = false;
            if (!playing) {
                // stop was called, stop the timer and end playback
                timer->stop();
                pb.stop(false);
            }
        } else {
            // audio buffer is full, up the timer interval so we can
            // write to the buffer next interval
            timer->setInterval(10);
            frameOffset += nwritten;
        }
    } else {
        // generate new frame
        synthMutex.lock();
        // execute 1 frame of the program
        runtime->step(synth, wtable);
        // synthesize the frame
        synth.fill(frame, FRAMESIZE);
        synthMutex.unlock();

        frameOffset = 0;
        frameOut = true;
        onTimeout(); // recurse to begin outputting the frame

    }
}
//
//void SynthWorker::run() {
//
//    if (runtime == nullptr) { // eh
//        return;
//    }
//
//    shouldStop = false;
//    runtime->reset();
//    
//    // begin playback
//    pb.start();
//
//    // frame buffer pointers
//    auto frame = buf.get();
//
//    bool stoppedEarly = false;
//    // loop
//    do {
//        if (runtime->isFinished()) {
//            if (loop) {
//                runtime->reset();
//            } else {
//                break;
//            }
//        }
//        
//        synthMutex.lock();
//        // execute 1 frame of the program
//        runtime->step(synth);
//        // synthesize the frame
//        synth.fill(frame, FRAMESIZE);
//
//        synthMutex.unlock();
//
//        // write it to the playback buffer
//        float *fp = frame;
//        size_t written = 0;
//        size_t toWrite = FRAMESIZE;
//        while ((written = pb.write(fp, toWrite)) != toWrite) {
//            toWrite -= written;
//            fp += written * 2;
//            mutex.lock();
//            //while (!pb.canWrite(toWrite)) {
//                writeAvailable.wait(&mutex, 10);
//            //}
//            stoppedEarly = shouldStop;
//            mutex.unlock();
//
//            if (stoppedEarly) {
//                break;
//            }
//
//        }
//        
//
//    } while (!stoppedEarly);
//
//    // if we stopped early, do not wait to playback whatever is in the buffer
//    pb.stop(!stoppedEarly);
//}
//
//void SynthWorker::setFrequency(uint16_t frequency) {
//    synthMutex.lock();
//    auto &cf = synth.getChannels();
//    cf.ch1.setFrequency(frequency);
//    cf.ch2.setFrequency(frequency);
//    cf.ch3.setFrequency(frequency);
//    synthMutex.unlock();
//    //pb.flush();
//}
//

//
//void SynthWorker::stop() {
//    mutex.lock();
//    shouldStop = true;
//    mutex.unlock();
//    // wake up this thread
//    writeAvailable.notify_all();
//}
//
//void SynthWorker::setLoop(bool _loop) {
//    loop = _loop;
//}



}
