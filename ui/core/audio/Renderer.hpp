
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
// Class handles all sound renderering. Sound is sent to the
// configured device set in Config. This class is intended to run in its own
// thread. Thus, all slots are thread-safe if invoked via signal-slot mechanism
//
class Renderer : public QObject {

    Q_OBJECT

public:

    struct Diagnostics {
        unsigned underruns;
        unsigned elapsed; // elapsed in milliseconds
        unsigned bufferUsage;
        unsigned bufferSize;
    };

    Renderer(QObject *parent = nullptr);
    ~Renderer();

    // DIAGNOSTICS ====

    //
    // Retrieves the current diagnostic state.
    //
    // Note: Function is thread-safe
    //
    Diagnostics diagnostics();

    //
    // Gets the current document.
    //
    // Note: Function is thread-safe
    //
    ModuleDocument* document();

    //
    // Gets the current document that is playing music. nullptr if not
    // playing music.
    //
    // Note: Function is thread-safe
    //
    ModuleDocument* documentPlayingMusic();

    //
    // Determines whether the renderer is enabled. The renderer is enabled if
    // the sound device was successfully configured. The renderer is disabled
    // if an audio error occurs during render.
    //
    // Note: Function is thread-safe
    //
    bool isEnabled();

    //
    // Determines if the renderer is renderering sound.
    //
    // Note: Function is thread-safe
    //
    bool isRunning();

    //
    // Gets a copy of the current engine frame.
    //
    // Note: Function is thread-safe
    //
    trackerboy::Engine::Frame currentFrame();

    //
    // Gets the last device error that occurred from device configuration or
    // error during render. MA_SUCCESS is returned on no error.
    //
    // Note: Function is thread-safe
    //
    ma_result lastDeviceError();

    //
    // Configures the output device with the given Sound config. If device
    // cannot be configured, the renderer is disabled. This function must
    // be called from the GUI thread.
    //
    // Note: Function is thread-safe
    //
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

    //
    // Sets pattern repeat mode. When enabled, the current playing pattern is
    // repeated.
    //
    void setPatternRepeat(bool repeat);

    // instrument preview
    void previewInstrument(quint8 note);

    //
    // Previews a note on the given track using the given instrument.
    //
    void previewNote(int note, int track, int instrument);

    // waveform preview
    void previewWaveform(quint8 note);

    void play();

    void playAtStart();

    void playFromCursor();

    void stepFromCursor();

    //
    // Stop previewing an instrument, waveform or row.
    //
    void stopPreview();

    //
    // Stops music playback by halting the engine
    //
    void stopMusic();

    void forceStop();
    
    //
    // Set the document to render. Any ongoing renders will use the old
    // document until stopped. All new renders will use this document
    //
    void setDocument(ModuleDocument *doc);

    //
    // stops all renders with the given document
    //
    void removeDocument(ModuleDocument *doc);

protected:

    virtual void timerEvent(QTimerEvent *evt) override;

private slots:

    //
    // Enables/Disables channel output for music playback
    //
    void setChannelOutput(ModuleDocument::OutputFlags flags);

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

    // returns true if the current thread is the same as this object's thread
    // if false is returned we need to use synchronization primitives
    bool isThreadSafe();

    // sets up the engine to play starting at the given pattern and row
    void playMusic(int pattern, int row, bool stepping = false);

    // utility function for preview slots
    void resetPreview();

    void previewNoteOrInstrument(int note, int track = -1, int instrument = -1);

    void setMusicDocument();

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

    //
    // Fills the playback buffer with newly renderered samples. Stops rendering
    // if there is no work to do and the buffer has drained completely.
    //
    void render();

    //
    // Immediately stops the render without letting the buffer drain.
    //
    void stopRender();

    // device callback functions

    static void deviceDataHandler(ma_device *device, void *out, const void *in, ma_uint32 frames);
    void _deviceDataHandler(int16_t *out, size_t frames);

    static void deviceStopHandler(ma_device *device);
    void _deviceStopHandler();

    // class members ---------------------------------------------------------

    QMutex mMutex;

    // the current document
    ModuleDocument *mDocument;
    ModuleDocument *mMusicDocument;

    ma_device_config mDeviceConfig;
    std::optional<ma_device> mDevice;

    int mTimerId;

    // playback buffer
    // written by the gui, read from the callback
    AudioRingbuffer mBuffer;

    ma_result mLastDeviceError;
    bool mEnabled;
    bool mRunning;
    // indicates if step mode is enabled
    bool mStepping;
    // determines if the engine should step (ignored when mStepping = false)
    bool mStep;

    int mPeriod;

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

    // when starting playback, delay by this number of samples
    // silence is played during this delay, which gives the renderer time
    // to fill the buffer
    size_t mPlaybackDelay;
    // ignores underruns when true, used when mState = State::stopping and we
    // are "draining" the buffer or letting it empty completely
    std::atomic_bool mDraining;
    // counter of underruns that have occurred. An underrun occurs when the
    // buffer does not have enough samples for the device.
    std::atomic_uint mUnderruns;
    // counter of total samples played out. This value is reset on every render.
    std::atomic_uint mSamplesElapsed;
    // buffer utilization at start of data callback
    std::atomic_uint mBufferUsage;
    std::atomic_uint mBufferSize;

};
