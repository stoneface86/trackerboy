
#pragma once

#include "core/audio/AudioStream.hpp"
#include "core/audio/Ringbuffer.hpp"
#include "core/audio/VisualizerBuffer.hpp"
#include "core/model/ModuleDocument.hpp"
#include "core/Config.hpp"
#include "core/FastTimer.hpp"

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
#include <chrono>
#include <memory>
#include <optional>

//
// Class handles all sound renderering. Sound is sent to the
// configured device set in Config. This class is intended to run in its own
// thread. Thus, all slots are thread-safe if invoked via signal-slot mechanism
//
class Renderer : public QObject {

    Q_OBJECT

    using Clock = std::chrono::steady_clock;

public:

    struct Diagnostics {
        int underruns;
        size_t bufferUse;
        size_t bufferSize;
        size_t writesSinceLastPeriod;
        Clock::duration lastPeriod;
        double elapsed;


    };

    Renderer(QObject *parent = nullptr);
    ~Renderer();

    // DIAGNOSTICS ====

    Diagnostics diagnostics();

    Guarded<VisualizerBuffer>& visualizerBuffer();

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
    //bool isEnabled();

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
    trackerboy::Frame currentFrame();

    //
    // Configures the output device with the given Sound config. If device
    // cannot be configured, the renderer is disabled. This function must
    // be called from the GUI thread.
    //
    // Note: Function is thread-safe
    //
    bool setConfig(Config::Sound const& config);

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

    void updateVisualizers();

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
    // Jumps to the given pattern if currently playing music
    //
    void jumpToPattern(int pattern);

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

    struct RenderContext {
        // the current document
        ModuleDocument *document;
        // the document that is playing music (usually the same as the current document)
        ModuleDocument *musicDocument;

        // indicates if step mode is enabled
        bool stepping;
        // determines if the engine should step (ignored when mStepping = false)
        bool step;

        trackerboy::Synth synth;
        trackerboy::GbApu apu;
        //trackerboy::RuntimeContext mRc;
        // read access to the current song, wave table and instrument table
        trackerboy::Engine engine;
        // has read access to an Instrument and wave table
        trackerboy::InstrumentPreview ip;


        PreviewState previewState;
        trackerboy::ChType previewChannel;

        trackerboy::Frame currentEngineFrame;

        State state;
        int stopCounter;

        // diagnostics
        Clock::time_point watchdog; // occurance of last watchdog reset
        Clock::time_point lastPeriod; // occurance of the last period
        Clock::duration periodTime; // time difference between the last period and the current one
        size_t writesSinceLastPeriod; // number of samples written for the last period

        RenderContext();
    };

    // type alias for mutually exclusive access to the RenderContext
    using Handle = Locked<RenderContext>;

    // sets up the engine to play starting at the given pattern and row
    void playMusic(Handle &handle, int pattern, int row, bool stepping = false);

    // utility function for preview slots
    void resetPreview(Handle &handle);

    void previewNoteOrInstrument(int note, int track = -1, int instrument = -1);

    void _setChannelOutput(Handle &handle, ModuleDocument::OutputFlags flags);

    // stream management -----------------------------------------------------

    //
    // Start the audio callback thread for the configured device. If the audio
    // callback thread is already running, the stop countdown is cancelled
    //
    void beginRender(Handle &handle);

    static void timerCallback(void *userData);

    //
    // Fills the playback buffer with newly renderered samples. Stops rendering
    // if there is no work to do and the buffer has drained completely.
    //
    void render();

    //
    // Immediately stops the render without letting the buffer drain.
    //
    void stopRender(Handle &handle, bool aborted = false);

    // class members ---------------------------------------------------------

    mutable QMutex mMutex;

    QThread mTimerThread;
    FastTimer *mTimer;

    AudioStream mStream;
    Guarded<VisualizerBuffer> mVisBuffer;

    //
    // All variables accessible from multiple threads are stored in the RenderContext
    // struct, access to them is guarded by a mutex.
    //
    Guarded<RenderContext> mContext;
    


};
