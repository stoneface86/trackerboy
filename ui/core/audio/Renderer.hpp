
#pragma once

#include "core/audio/RenderFrame.hpp"
#include "core/audio/Ringbuffer.hpp"
#include "core/model/ModuleDocument.hpp"
#include "core/Config.hpp"
#include "core/Miniaudio.hpp"

#include "trackerboy/data/Song.hpp"
#include "trackerboy/data/Instrument.hpp"
#include "trackerboy/data/Waveform.hpp"
#include "trackerboy/engine/Engine.hpp"
#include "trackerboy/InstrumentPreview.hpp"
#include "trackerboy/Synth.hpp"
#include "trackerboy/note.hpp"

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QWaitCondition>

#include <cstddef>
#include <memory>
#include <optional>

//
// Class handles all sound renderering. Sound is either sent to the
// configured device set in Config or is written to file (export, not implemented yet)
//
class Renderer : public QObject {

    Q_OBJECT

public:

    struct Diagnostics {
        unsigned lockFails;
        unsigned underruns;
        unsigned elapsed;
        unsigned bufferUsage;
        unsigned bufferSize;
    };

    Renderer(QObject *parent = nullptr);
    ~Renderer();

    // DIAGNOSTICS ====

    Diagnostics diagnostics();

    ma_device const& device();

    ModuleDocument* document();

    //ModuleDocument* documentPlayingMusic();

    bool isEnabled();

    bool isRunning();

    ma_result lastDeviceError();

    void setConfig(Miniaudio &miniaudio, Config::Sound const& config);

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
    // An error occurred during audio playback.
    //
    void audioError();

    //
    // emitted when a new frame is rendererd
    //
    void frameSync();

public slots:

    void clearDiagnostics();

    // to be called periodically when the audioStarted signal was emitted.
    // connect this slot to MainWindow's update timer timeout signal
    void render();

    //void stopMusic();

    // instrument preview
    void previewInstrument(quint8 note);

    // waveform preview
    void previewWaveform(quint8 note);

    //void play();

    //void playPattern();

    //void playFromCursor();

    //
    // Begin playing the song from the beginning
    //
    //void playFromStart();

    //
    // Stop previewing an instrument, waveform or row.
    //
    void stopPreview();
    
    //
    // Set the document to render. Any ongoing renders will use the old
    // document until stopped. All new renders will use this document
    //
    void setDocument(ModuleDocument *doc);

private:
    Q_DISABLE_COPY(Renderer)

    enum class PreviewState {
        none,
        waveform,
        instrument
    };

    enum class State {
        running,    // render samples
        stopping,   // no longing synthesizing, transitions to stopped when the buffer empties
        stopped     // no longer renderering anything, do nothing when render is called
    };


    // utility function for preview slots
    void resetPreview();


    // device management -----------------------------------------------------

    //
    // Destroys the current device if initialized
    //
    void closeDevice();

    //
    // Start the audio callback thread for the configured device. If the audio
    // callback thread is already running, the stop countdown is cancelled
    //
    void beginRender();

    static void deviceDataHandler(ma_device *device, void *out, const void *in, ma_uint32 frames);
    void _deviceDataHandler(int16_t *out, size_t frames);

    static void deviceStopHandler(ma_device *device);
    void _deviceStopHandler();

    // class members ---------------------------------------------------------

    QMutex mMutex;

    // the current document
    ModuleDocument *mDocument;

    ma_device_config mDeviceConfig;
    std::optional<ma_device> mDevice;

    // playback buffer
    // written by the gui, read from the callback
    AudioRingbuffer mBuffer;

    // return buffer or the "played out" buffer
    // this buffer contains audio that has already been sent to the speakers
    // written by the callback and is to be read by the GUI for visualizers
    //AudioRingbuffer mReturnBuffer;

    ma_result mLastDeviceError;

    bool mEnabled;

    trackerboy::Synth mSynth;
    trackerboy::GbApu mApu;
    //trackerboy::RuntimeContext mRc;
    // read access to the current song, wave table and instrument table
    trackerboy::Engine mEngine;
    // has read access to an Instrument and wave table
    trackerboy::InstrumentPreview mIp;


    PreviewState mPreviewState;
    trackerboy::ChType mPreviewChannel;

    trackerboy::Engine::Frame mCurrentEngineFrame;

    State mState;
    int mStopCounter;

    // callback flags

    std::atomic_bool mDraining;
    std::atomic_uint mUnderruns;
    std::atomic_uint mSamplesElapsed;
    std::atomic_uint mBufferUsage;

};
