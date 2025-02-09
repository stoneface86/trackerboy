
#pragma once

#include "audio/AudioStream.hpp"
#include "audio/AudioEnumerator.hpp"
#include "audio/VisualizerBuffer.hpp"
#include "config/data/SoundConfig.hpp"
#include "core/ChannelOutput.hpp"
#include "utils/FastTimer.hpp"
#include "core/Module.hpp"
#include "utils/Guarded.hpp"

#include "trackerboy/apu/DefaultApu.hpp"
#include "trackerboy/data/Song.hpp"
#include "trackerboy/data/Instrument.hpp"
#include "trackerboy/data/Waveform.hpp"
#include "trackerboy/engine/Engine.hpp"
#include "trackerboy/InstrumentPreview.hpp"
#include "trackerboy/Synth.hpp"
#include "trackerboy/note.hpp"

#include <QObject>
#include <QThread>

#include <chrono>

//
// Class handles all sound renderering. Sound is sent to the
// configured device set in Config.
//
class Renderer : public QObject {

    Q_OBJECT

    using Clock = std::chrono::steady_clock;

public:

    struct BufferStats {
        // usage, in number of samples, of the buffer
        int usage;
        // capacity, in number of samples, of the buffer
        int capacity;
        // number of samples written during the last period
        int writesSinceLastPeriod;
        // duration of the last period, in milliseconds
        double lastPeriodMs;
    };

    explicit Renderer(Module &mod, QObject *parent = nullptr);
    ~Renderer();

    // Statistics ===

    //
    // Returns the total count of underruns that have occurred from all
    // renders.
    //
    unsigned statUnderruns() const;

    //
    // Gets the current buffer statistics.
    //
    BufferStats statBuffer();

    //
    // Gets the elapsed time, in milliseconds, of the current render. Behavior
    // is undefined when isRunning() is false.
    //
    long statElapsed() const;

    //
    // Get the current samplerate
    //
    int samplerate();

    //
    // Accessor for the visualizer buffer. The updateVisualizers() signal is
    // emitted when this buffer is modified.
    //
    Guarded<VisualizerBuffer>& visualizerBuffer();

    //
    // Determines if the renderer is renderering sound.
    //
    bool isRunning();

    //
    // Determines if the renderer is in step-mode
    //
    bool isStepping();

    //
    // Determines if the renderer is playing music
    //
    bool isPlaying();

    //
    // Gets a copy of the current engine frame.
    //
    trackerboy::Frame currentFrame();

    //
    // Configures the output device with the given Sound config. If device
    // cannot be configured, the renderer is disabled. This function must
    // be called from the GUI thread.
    //
    bool setConfig(SoundConfig const& config, AudioEnumerator const& enumerator);

    //
    // Changes the note being previewed for an instrument/waveform preview.
    // If there is no current preview this function does nothing.
    //
    void setPreviewNote(int note);

    //
    // Begins renderering an instrument or note preview. For note previews,
    // track must be 0-3. For instrument previews, track must be -1 and
    // instrument must be between 0 and 63 (instrument id).
    //
    void instrumentPreview(int note, int track, int instrument);

    //
    // Begins renderering a waveform preview. CH3 is unlocked and loaded with
    // the given waveform using the waveId.
    //
    void waveformPreview(int note, int waveId);

    //
    // Update the framerate used by the synth. Call this when the module's framerate
    // changes.
    //
    void updateFramerate();

    //
    // Clears counters in the diagnostic data
    //
    void clearDiagnostics();

    //
    // Sets pattern repeat mode. When enabled, the current playing pattern is
    // repeated.
    //
    void setPatternRepeat(bool repeat);

    //
    // Begin playing music from the current pattern and row. stepmode determines
    // if the renderer will "step" rows.
    //
    void play(int pattern, int row, bool stepmode);

    //
    // If the renderer is in step mode, the next row will be stepped on next
    // render call.
    //
    void stepNextFrame();

    //
    // Takes the renderer out of step mode and plays music normally.
    //
    void stepOut();

    //
    // Jumps to the given pattern if currently playing music
    //
    void jumpToPattern(int pattern);

    //
    // Stop previewing an instrument, waveform or row.
    //
    void stopPreview();

    //
    // Stops music playback by halting the engine. Sound playback will cease
    // once the buffer has drained.
    //
    void stopMusic();

    //
    // Force stop of the current renderer immediately. Rendered audio to be
    // played out is lost.
    //
    void forceStop();

    //
    // Resets the APU volume setting (NR50).
    //
    void resetGlobalVolume();

    void setChannelOutput(ChannelOutput::Flags output);

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
    // Emitted when music has started/stopped playing
    //
    void isPlayingChanged(bool playing);

    //
    // An error occurred during audio playback.
    //
    void audioError();

    //
    // emitted when a new frame is renderered
    //
    void frameSync();

    //
    // Emitted when the visualizer buffer has been modified
    //
    void updateVisualizers();

private:

    //
    // invoked when the current song being edited has changed. If music is
    // currently playing, the renderer will begin playing on the new song.
    //
    void setSong();

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

    //
    // This struct contains shared data between the GUI thread and the
    // render thread.
    //
    struct RenderContext {
        // the current module
        Module &mod;

        // indicates if step mode is enabled
        bool stepping;
        // determines if the engine should step (ignored when mStepping = false)
        bool step;

        std::shared_ptr<trackerboy::Song> song;

        trackerboy::DefaultApu apu;
        trackerboy::Synth synth;
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

        size_t bufferSize; // cache this here so we don't have to call mStream.bufferSize() in the render thread

        // diagnostics
        Clock::time_point watchdog; // occurance of last watchdog reset
        Clock::time_point lastPeriod; // occurance of the last period
        Clock::duration periodTime; // time difference between the last period and the current one
        size_t writesSinceLastPeriod; // number of samples written for the last period

        RenderContext(Module &mod);
    };

    // type alias for mutually exclusive access to the RenderContext
    using Handle = Locked<RenderContext>;

    // sets up the engine to play starting at the given pattern and row
    void _play(Handle &handle, int pattern, int row, bool stepping = false);

    void _stopMusic(Handle &handle);

    // utility function for preview slots
    void resetPreview(Handle &handle);

    void previewNoteOrInstrument(int note, int track = -1, int instrument = -1);

    void _setChannelOutput(Handle &handle, ChannelOutput::Flags flags);

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
    // This function is called periodically from a separate thread.
    //
    void render();

    //
    // Immediately stops the render without letting the buffer drain.
    //
    void stopRender(Handle &handle, bool aborted = false);

    // class members ---------------------------------------------------------

    QThread mTimerThread;
    FastTimer *mTimer;      // thread-safe: yes

    AudioStream mStream;    // thread-safe: no
    Guarded<VisualizerBuffer> mVisBuffer;

    ChannelOutput::Flags mOutputFlags;

    Clock::time_point mRenderStartTime;

    //
    // All variables accessible from multiple threads are stored in the RenderContext
    // struct, access to them is guarded by a mutex.
    //
    Guarded<RenderContext> mContext;
    


};
