
#include "Renderer.hpp"

#include <QTime>

#include <chrono>
#include <thread>


Renderer::Renderer(ModuleDocument &document, InstrumentListModel &instrumentModel, WaveListModel &waveModel, QObject *parent) :
    QObject(parent),
    mAudioStopCondition(),
    mRendererThread(document, instrumentModel, waveModel, mAudioStopCondition),
    mMutex(),
    mRunning(true),
    mStopBackground(false),
    mBackgroundThread(nullptr)
{
    mBackgroundThread = QThread::create(&Renderer::backgroundThreadRun, this);
    connect(mBackgroundThread, &QThread::finished, mBackgroundThread, &QThread::deleteLater);
    mBackgroundThread->start();
}

Renderer::~Renderer() {
    mMutex.lock();
    mStopBackground = true;
    mMutex.unlock();

    mAudioStopCondition.wakeAll();
}

void Renderer::play() {
    mRendererThread.command(RendererThread::Command::play);
}

void Renderer::playPattern() {
    mRendererThread.command(RendererThread::Command::playPattern);
}

void Renderer::playFromStart() {
    mRendererThread.command(RendererThread::Command::playFromStart);
}

void Renderer::playFromCursor() {
    mRendererThread.command(RendererThread::Command::playFromCursor);
}

void Renderer::previewWaveform(trackerboy::Note note) {
    mRendererThread.command(RendererThread::Command::previewWave, note);   
}

void Renderer::previewInstrument(trackerboy::Note note) {
    mRendererThread.command(RendererThread::Command::previewInst, note);
}

void Renderer::stopPreview() {
    mRendererThread.command(RendererThread::Command::previewStop);
}

void Renderer::stopMusic() {
    mRendererThread.command(RendererThread::Command::stop);
}

void Renderer::stop() {
    mRendererThread.command(RendererThread::Command::stop);
}

void Renderer::setConfig(Config::Sound const &config) {



    // whatever is queued will be lost
    mRendererThread.stop();
    
    mRendererThread.applyConfig(config);

    //mSynth.setSamplingRate(audio::SAMPLERATE_TABLE[sound.samplerate]);*/
    ////mSynth.setBass(sound.bassFrequency);
    ////mSynth.setTreble(sound.treble, sound.trebleFrequency);

    ////double gain = pow(2.0, sound.volume / 6.0);
    ////mSynth.setVolume(gain);

    //mSynth.setupBuffers();

    //mPb.open();
    ////mPb.setLatency(50);
    //start(QThread::Priority::TimeCriticalPriority);
}

void Renderer::backgroundThreadRun(Renderer *renderer) {
    renderer->handleBackground();
}

void Renderer::handleBackground() {
    // the audio callback will signal the condition variable when there's
    // nothing to play.

    for (;;) {
        mMutex.lock();
        if (mStopBackground) {
            mMutex.unlock();
            break;
        }
        mAudioStopCondition.wait(&mMutex);
        mMutex.unlock();

        mRendererThread.stop();
    }
}
