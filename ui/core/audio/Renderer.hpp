
#pragma once

#include "core/audio/RenderBuffer.hpp"
#include "core/audio/Ringbuffer.hpp"
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
#include <chrono>
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

    //
    // Time since the last sync event in nanoseconds
    //
    long long lastSyncTime();

    ma_device const& device() const;

    //
    // Get the audio return buffer. Note: only the GUI thread may read this
    // buffer and only the callback thread may write to it.
    //
    AudioRingbuffer& returnBuffer();

    //
    // To be called during audioSync(). Gets the current frame being played.
    // Call releaseFrame() when finished accessing.
    //
    RenderFrame const& acquireCurrentFrame();

    //
    // Releases the previously acquired frame returned from acquireCurrentFrame.
    // The reference to that frame is now invalid.
    //
    void releaseFrame();

    bool isRunning();

    void setConfig(Config::Sound const& config);

    void playMusic(uint8_t orderNo, uint8_t rowNo);
    
    // row preview (just the track)
    //void preview(trackerboy::TrackRow const& row);

    // row preview (all channels)
    //void preview(trackerboy::PatternRow const& row);

signals:

    //
    // Signal emitted when the audio callback thread was started
    //
    void audioStarted();

    //
    // Signal emitted when the audio callback thread was stopped.
    //
    void audioStopped();

    //
    // Audio synchronization. Emitted when the callback has finished
    // rendering a block of audio data to the device. The size of this block
    // can vary between syncs and its size depends on latency settings.
    //
    // The lower the latency, the more often this signal is emitted.
    //
    void audioSync();


public slots:

    void clearDiagnostics();

    void stopMusic();

    // instrument preview
    void previewInstrument(trackerboy::Note note);

    // waveform preview
    void previewWaveform(trackerboy::Note note);

    void play();

    void playPattern();

    void playFromCursor();

    void playFromStart();

    void stopPreview();

private:

    using Clock = std::chrono::steady_clock;

    enum class PreviewState {
        none,
        waveform,
        instrument
    };

    // utility function for preview slots
    void resetPreview();

    // device management -----------------------------------------------------

    void closeDevice();

    //
    // Start the audio callback thread for the configured device. If the audio
    // callback thread is already running, the stop countdown is cancelled
    //
    void startDevice();

    void stopDevice();


    // thread main functions -------------------------------------------------

    static void audioCallbackRun(ma_device *device, void *out, const void *in, ma_uint32 frames);
    void handleCallback(int16_t *out, size_t frames);

    static void backgroundThreadRun(Renderer *renderer);
    void handleBackground();

    // class members ---------------------------------------------------------

    Miniaudio &mMiniaudio;
    Spinlock &mSpinlock;

    InstrumentListModel &mInstrumentModel;
    SongListModel &mSongModel;
    WaveListModel &mWaveModel;

    QWaitCondition mIdleCondition;
    QWaitCondition mCallbackCondition;
    QMutex mCallbackMutex;
    QMutex mMutex;
    QThread *mBackgroundThread;

    bool mRunning;          // is the audio callback thread active?
    bool mStopBackground;   // flag to stop the background thread
    bool mStopDevice;
    bool mSync;
    //std::atomic_bool mStopDevice;       // flag to stop the callback thread
    //std::atomic_bool mSync;             // sync audio?
    std::atomic_bool mCancelStop;
    std::optional<ma_device> mDevice;

    // diagnostic info
    std::atomic_uint mLockFails;        // number of spinlock failures
    std::atomic_uint mUnderruns;        // number of underruns, or failure to output samples
    std::atomic_uint mSamplesElapsed;   // number of samples written to be played out
    std::atomic_uint mBufferUsage;      // number of frames queued in the buffer
    Clock::duration mLatency;
    
    trackerboy::Synth mSynth;
    trackerboy::RuntimeContext mRc;
    // read access to the current song, wave table and instrument table
    trackerboy::Engine mEngine;
    // has read access to an Instrument and wave table
    trackerboy::InstrumentRuntime mIr;

    RenderBuffer mBuffer;

    PreviewState mPreviewState;
    trackerboy::ChType mPreviewChannel;

        
    int mStopCounter;
    bool mShouldStop;
    bool mStopped;

    size_t mSyncCounter;
    size_t mSyncPeriod;

    AudioRingbuffer mReturnBuffer;

    

    RenderFrame mLastFrameOut;
    Spinlock mFrameLock;

};
