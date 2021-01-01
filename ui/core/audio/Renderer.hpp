
#pragma once

#include "core/audio/RenderBuffer.hpp"
#include "core/model/InstrumentListModel.hpp"
#include "core/model/SongListModel.hpp"
#include "core/model/WaveListModel.hpp"
#include "core/Config.hpp"
#include "core/Miniaudio.hpp"
#include "core/Spinlock.hpp"

#include "trackerboy/data/Song.hpp"
#include "trackerboy/data/Instrument.hpp"
#include "trackerboy/data/Waveform.hpp"
#include "trackerboy/engine/Engine.hpp"
#include "trackerboy/engine/InstrumentRuntime.hpp"
#include "trackerboy/Synth.hpp"
#include "trackerboy/note.hpp"

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QWaitCondition>

#include <atomic>
#include <optional>


class Renderer : public QObject {

    Q_OBJECT

public:

    Renderer(
        Miniaudio &miniaudio, 
        Spinlock &spinlock,
        trackerboy::InstrumentTable &itable,
        trackerboy::WaveTable &wtable,
        InstrumentListModel &instrumentModel,
        SongListModel &songModel,
        WaveListModel &waveModel
    );
    ~Renderer();

    // DIAGNOSTICS ====

    //
    // gets the current count of lock failures, or when the callback thread failed
    // to lock the spinlock.
    //
    unsigned lockFails() const;

    //
    // total count of underruns that have occurred
    //
    unsigned underruns() const;

    //
    // time elapsed in milleseconds of the current render
    //
    unsigned elapsed() const;

    //
    // buffer utilization in whole frames.
    //
    unsigned bufferUsage() const;

    //
    // Size of the buffer in whole frames.
    //
    unsigned bufferSize() const;

    ma_device const& device() const;

    bool isRunning();

    void setConfig(Config::Sound const& config);

    void playMusic(trackerboy::Song *song, uint8_t orderNo, uint8_t rowNo);
    
    // row preview (just the track)
    //void preview(trackerboy::TrackRow const& row);

    // row preview (all channels)
    //void preview(trackerboy::PatternRow const& row);

public slots:

    void stopMusic();

    // instrument preview
    void previewInstrument(trackerboy::Note note);

    // waveform preview
    void previewWaveform(trackerboy::Note note);

    

    void stopPreview();

private:

    enum class PreviewState {
        none,
        waveform,
        instrument
    };

    void closeDevice();

    //
    // Start the audio callback thread for the configured device
    //
    void startDevice();

    void resetPreview();


    void handleBackground();
    static void backgroundThreadRun(Renderer *renderer);

    Miniaudio &mMiniaudio;
    Spinlock &mSpinlock;

    QWaitCondition mAudioStopCondition;
    QMutex mMutex;
    QThread *mBackgroundThread;

    bool mRunning;          // is the audio callback thread active?
    bool mStopBackground;   // flag to stop the background thread


    std::optional<ma_device> mDevice;

    // ~~~~ AUDIO CALLBACK 

    static void audioCallbackRun(ma_device *device, void *out, const void *in, ma_uint32 frames);

    void handleCallback(int16_t *out, size_t frames);

    trackerboy::Synth mSynth;
    trackerboy::RuntimeContext mRc;
    // read access to the current song, wave table and instrument table
    trackerboy::Engine mEngine;
    // has read access to an Instrument and wave table
    trackerboy::InstrumentRuntime mIr;

    InstrumentListModel &mInstrumentModel;
    SongListModel &mSongModel;
    WaveListModel &mWaveModel;

    RenderBuffer mBuffer;

    PreviewState mPreviewState;
    trackerboy::ChType mPreviewChannel;

    int mStopCounter;
    bool mStopped;

    // diagnostic info
    std::atomic_uint mLockFails;        // number of spinlock failures
    std::atomic_uint mUnderruns;        // number of underruns, or failure to output samples
    std::atomic_uint mSamplesElapsed;   // number of samples written to be played out
    std::atomic_uint mBufferUsage;      // number of frames queued in the buffer

};
