
#pragma once

#include "core/audio/AudioOutStream.hpp"
#include "core/model/InstrumentListModel.hpp"
#include "core/model/SongListModel.hpp"
#include "core/model/WaveListModel.hpp"
#include "core/Config.hpp"
#include "core/Miniaudio.hpp"

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

#include <cstddef>
#include <memory>


class Renderer : public QObject {

    Q_OBJECT

public:

    struct Diagnostics {
        unsigned underruns;
        unsigned elapsed;
        size_t bufferUsage;
        size_t bufferSize;
        long long lastSyncTime;
    };


    Renderer(
        Miniaudio &miniaudio,
        ModuleDocument &document,
        InstrumentListModel &instrumentModel,
        SongListModel &songModel,
        WaveListModel &waveModel
    );
    ~Renderer();

    // DIAGNOSTICS ====

    Diagnostics diagnostics();

    ma_device const& device() const;

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
    // A frame of audio data has been synthesized and is buffered to be played out
    // The GUI should update tracker position, visualizers, etc based on this new frame
    //
    void frameSync();


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

    enum class PreviewState {
        none,
        waveform,
        instrument
    };

    // utility function for preview slots
    void resetPreview();

    // render audio data to the given buffer
    void render(AudioRingbuffer::Writer &buffer);

    // device management -----------------------------------------------------

    void closeDevice();

    //
    // Start the audio callback thread for the configured device. If the audio
    // callback thread is already running, the stop countdown is cancelled
    //
    void beginRender();

    // thread main functions -------------------------------------------------

    static void backgroundThreadRun(Renderer *renderer);
    void handleBackground();

    // class members ---------------------------------------------------------

    Miniaudio &mMiniaudio;
    ModuleDocument &mDocument;

    InstrumentListModel &mInstrumentModel;
    SongListModel &mSongModel;
    WaveListModel &mWaveModel;

    QWaitCondition mIdleCondition;
    QMutex mMutex;
    std::unique_ptr<QThread> mBackgroundThread;

    AudioOutStream mStream;

    bool mRunning;              // is the audio callback thread active?
    bool mStopBackground;       // flag to stop the background thread
    bool mStopDevice;           // flag to stop the callback thread
    std::unique_ptr<ma_device> mDevice;


    // renderering
    
    trackerboy::Synth mSynth;
    trackerboy::RuntimeContext mRc;
    // read access to the current song, wave table and instrument table
    trackerboy::Engine mEngine;
    // has read access to an Instrument and wave table
    trackerboy::InstrumentRuntime mIr;


    PreviewState mPreviewState;
    trackerboy::ChType mPreviewChannel;

    size_t mSynthBufferRemaining;
    int16_t *mSynthBuffer;

    int mStopCounter;

};
