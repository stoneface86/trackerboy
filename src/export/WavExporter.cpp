
#include "export/WavExporter.hpp"

#include "audio/Wav.hpp"

#include <QDir>
#include <QFileInfo>

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
    mChannels(ChannelOutput::AllOn),
    mSeparate(false),
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

void WavExporter::setChannels(ChannelOutput::Flags channels) {
    mChannels = channels;
}

void WavExporter::setSeparate(bool separate) {
    mSeparate = separate;
}

void WavExporter::setSeparatePrefix(QString const& prefix) {
    mSeparatePrefix = prefix;
}

#define TU WavExporterTU
namespace TU {

struct Batch {
    QString filename;
    ChannelOutput::Flags channels;
};

}


void WavExporter::run() {

    // batches for this run
    std::array<TU::Batch, 4> batches;
    int batchCount = 0;
    if (mSeparate) {
        // separate channel per file, each channel gets its own batch
        auto iter = batches.begin();

        QDir dest(mDestination);

        for (int i = 0; i < 4; ++i) {
            auto const flag = (ChannelOutput::Flag)(1 << i);
            if (mChannels.testFlag(flag)) {
                iter->channels = flag;
                iter->filename = dest.filePath(
                    QStringLiteral("%1.ch%2.wav").arg(
                        mSeparatePrefix,
                        QString::number(i + 1)
                    ));
                ++iter;
            }
        }

        batchCount = iter - batches.begin();
    } else {
        // one file, one batch
        batchCount = 1;
        batches[0].filename = mDestination;
        batches[0].channels = mChannels;
    }

    // temporary buffer for transferring samples from apu to the wav file
    auto buffersize = mSynth.framesize() * 2;
    auto buffer = std::make_unique<float[]>(buffersize);

    for (int i = 0; i < batchCount; ++i) {

        trackerboy::Player player(mEngine);
        player.start(mDuration);

        auto const channels = batches[i].channels;
        for (int ch = 0; ch < 4; ++ch) {
            if (channels.testFlag((ChannelOutput::Flag)(1 << ch))) {
                mEngine.lock(static_cast<trackerboy::ChType>(ch));
            } else {
                mEngine.unlock(static_cast<trackerboy::ChType>(ch));
            }
        }


        Wav wav(batches[i].filename.toStdString(), 2, mSamplerate);
        if (!wav.stream().good()) {
            mFailed = true;
            return;
        }

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
}
