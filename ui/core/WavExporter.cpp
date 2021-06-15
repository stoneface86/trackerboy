
#include "core/WavExporter.hpp"

#include "miniaudio.h"


WavExporter::WavExporter(
    trackerboy::Module const& mod,
    unsigned samplerate,
    QObject *parent
) :
    QThread(parent),
    mSamplerate(samplerate),
    mSynth(samplerate, mod.framerate()),
    mApu(mSynth.apu()),
    mEngine(mApu, &mod),
    mDuration(0),
    mDestination(),
    mFailed(false),
    mAbort(false)
{
    mSynth.apu().setVolume(0.707f);
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

    ma_encoder_config config = ma_encoder_config_init(ma_resource_format_wav, ma_format_s16, 2, mSamplerate);
    ma_encoder encoder;

    auto dest = mDestination.toLatin1();
    auto result = ma_encoder_init_file(dest.data(), &config, &encoder);
    if (result != MA_SUCCESS) {
        mFailed = true;
        return;
    }

    // temporary buffer for transferring samples from apu to the wav file
    std::vector<int16_t> buffer;
    buffer.resize(mSynth.framesize() * 2);

    emit progressMax(player.progressMax());
    auto lastProgress = player.progress();
    emit progress(lastProgress);
    auto &apu = mSynth.apu();

    while (player.isPlaying()) {

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
        mSynth.run();

        auto samplesRead = apu.readSamples(buffer.data(), buffer.size());

        size_t toWrite = samplesRead;
        auto dataPtr = buffer.data();
        while (toWrite) {
            auto written = ma_encoder_write_pcm_frames(&encoder, dataPtr, toWrite);
            toWrite -= written;
            dataPtr += written * 2;
        }


    }

    ma_encoder_uninit(&encoder);
    mFailed = false;
    
}
