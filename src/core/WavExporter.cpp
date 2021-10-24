
#include "core/WavExporter.hpp"

#include "core/audio/Wav.hpp"

#include <memory>


WavExporter::WavExporter(
    Module const& mod,
    int samplerate,
    QObject *parent
) :
    QThread(parent),
    mSamplerate(samplerate),
    mApu(),
    mSynth(mApu, samplerate, mod.data().framerate()),
    mEngine(mApu, &mod.data()),
    mDuration(0),
    mDestination(),
    mFailed(false),
    mAbort(false)
{
    mEngine.setSong(mod.song());
}

void WavExporter::setDuration(trackerboy::Player::Duration duration) {
    mDuration = duration;
}

void WavExporter::setDestination(QString const& dest) {
    mDestination = dest;
}

bool WavExporter::failed() const {
    return mFailed;
}

void WavExporter::cancel() {
    QMutexLocker locker(&mMutex);
    mAbort = true;
}


void WavExporter::run() {
    
    trackerboy::Player player(mEngine);
    player.start(mDuration);

    Wav wav(mDestination.toStdString(), 2, mSamplerate);
    if (!wav.stream().good()) {
        mFailed = true;
        return;
    }

    // temporary buffer for transferring samples from apu to the wav file
    auto buffersize = mSynth.framesize() * 2;
    auto buffer = std::make_unique<float[]>(buffersize);

    emit progressMax(player.progressMax());
    auto lastProgress = player.progress();
    emit progress(lastProgress);

    for (;;) {

        mMutex.lock();
        if (mAbort) {
            mAbort = false;
            mMutex.unlock();
            break;
        }
        mMutex.unlock();

        auto currentProgress = player.progress();
        if (currentProgress != lastProgress) {
            lastProgress = currentProgress;
            emit progress(currentProgress);
        }

        player.step();
        if (!player.isPlaying()) {
            break;
        }
        mSynth.run();

        auto samplesRead = mApu.readSamples(buffer.get(), mSynth.framesize());
        wav.write(buffer.get(), samplesRead);
        if (!wav.stream().good()) {
            mFailed = true;
            return;
        }

    }

    mFailed = false;
    
}
