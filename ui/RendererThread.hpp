
#pragma once

#include "model/ModuleDocument.hpp"
#include "model/InstrumentListModel.hpp"
#include "model/WaveListModel.hpp"
#include "Config.hpp"

#include "trackerboy/engine/Engine.hpp"
#include "trackerboy/engine/InstrumentRuntime.hpp"
#include "trackerboy/synth/Synth.hpp"

#include "miniaudio.h"

#include <QWaitCondition>

#include <optional>

//
// This class isn't a thread itself, but rather manages the audio callback thread
// used by the Renderer.
//
class RendererThread {

public:

    enum class Command : uint8_t {
        none,
        previewWave,
        previewInst,
        previewStop,
        play,
        playPattern,
        playFromStart,
        playFromCursor,
        stop
    };

    RendererThread(ModuleDocument &document,
                   InstrumentListModel &instrumentModel,
                   WaveListModel &waveModel,
                   QWaitCondition &stopCondition
                   );

    ~RendererThread();

    void applyConfig(Config::Sound const &config);

    void close();

    void stop();

    //
    // Send a command to the callback thread.
    //
    void command(Command cmd, uint8_t data = 0);

private:

    enum class PreviewState {
        none,
        waveform,
        instrument
    };

    QWaitCondition &mStopCondition;

    // mDevice is initialized if the optional has a value
    std::optional<ma_device> mDevice;
    QMutex mMutex;
    bool mRunning;

    std::atomic_uint16_t mCommand;

    // ------------------------------------------------------------------
    // NO TOUCHING: Members below this line CAN ONLY be modified by the callback thread

    static void audioCallback(ma_device *device, void *out, const void *in, ma_uint32 frames);

    void handleCallback(int16_t *out, size_t frames);

    void resetPreview();


    // == spinlock required! use mDocument.trylock()
    ModuleDocument &mDocument;
    InstrumentListModel &mInstrumentModel;
    WaveListModel &mWaveModel;

    trackerboy::Synth mSynth;
    trackerboy::RuntimeContext mRc;
    // read access to the current song, wave table and instrument table
    trackerboy::Engine mEngine;
    // has read access to an Instrument and wave table
    trackerboy::InstrumentRuntime mIr;
    // == end of spinlock requirement

    // a spinlock should be used before reading model data, but do not block
    // trylock the spinlock and on failure output nothing

    PreviewState mPreviewState;
    trackerboy::ChType mPreviewChannel;

    int mStopCounter;
    bool mStopped;
    bool mMusicPlaying;

    int16_t *mFrameBuffer;
    // remaining samples in frame buffer
    size_t mFrameRemaining;
    
};

