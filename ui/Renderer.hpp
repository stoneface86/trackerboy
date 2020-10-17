
#pragma once

#include "model/ModuleDocument.hpp"
#include "model/InstrumentListModel.hpp"
#include "model/WaveListModel.hpp"

#include "audio.hpp"
#include "trackerboy/engine/Engine.hpp"
#include "trackerboy/engine/InstrumentRuntime.hpp"
#include "trackerboy/synth/Synth.hpp"
#include "trackerboy/note.hpp"

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QWaitCondition>

#include <cstdint>

//
// Renderer class handles pattern playback and instrument/waveform previews
//
class Renderer : public QThread {

    Q_OBJECT

public:

    Renderer(ModuleDocument &document, InstrumentListModel &instrumentModel, WaveListModel &waveModel, QObject *parent = nullptr);
    ~Renderer();

    void setDevice(struct SoundIoDevice *device, audio::Samplerate samplerate);

public slots:
    void play();
    void playPattern();
    void playFromStart();
    void playFromCursor();

    void previewWaveform(trackerboy::Note note);

    void previewInstrument(trackerboy::Note note);

    void stopPreview();

    void stopMusic();

protected:
    void run() override;

signals:
    void playing(); // emitted when music starts playing
    void stopped(); // emitted when music stops playing via halt effect or by user action

private:
    void resetPreview();

    void stopIdling();

    bool hasNoWork();

    ModuleDocument &mDocument;
    InstrumentListModel &mInstrumentModel;
    WaveListModel &mWaveModel;
    audio::PlaybackQueue mPb;
    trackerboy::Synth mSynth;
    trackerboy::Engine mEngine;
    trackerboy::InstrumentRuntime mIr;

    enum class PreviewState {
        none,
        waveform,
        instrument
    };

    PreviewState mPreviewState;
    trackerboy::ChType mPreviewChannel;

    //bool mRendering;
    bool mMusicPlaying;
    bool mRunning;
    bool mIdling;

    QMutex mMutex;
    QMutex mIdlingMutex;
    QWaitCondition mIdlingCond;

};
