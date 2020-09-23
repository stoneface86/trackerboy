
#pragma once

#include <QObject>
#include <QMutex>

#include "model/ModuleDocument.hpp"
#include "model/InstrumentListModel.hpp"
#include "model/WaveListModel.hpp"

#include "audio.hpp"
#include "trackerboy/engine/Engine.hpp"
#include "trackerboy/engine/InstrumentRuntime.hpp"
#include "trackerboy/synth/Synth.hpp"
#include "trackerboy/note.hpp"


class RenderWorker : public QObject {

    Q_OBJECT

public:
    RenderWorker(ModuleDocument &document, InstrumentListModel &instrumentModel, WaveListModel &waveModel);

    void previewInstrument(trackerboy::Note note);

    void previewWaveform(trackerboy::Note note);

    void stopPreview();

    bool isRendering();

    void quitRender();

public slots:
    void render();

private:
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

    bool mRendering;
    bool mMusicPlaying;

    QMutex mMutex;

};
