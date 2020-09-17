
#pragma once

#include <QObject>
#include <QMutex>

#include "model/ModuleDocument.hpp"

#include "audio.hpp"
#include "trackerboy/engine/Engine.hpp"
#include "trackerboy/synth/Synth.hpp"
#include "trackerboy/note.hpp"


class RenderWorker : public QObject {

    Q_OBJECT

public:
    RenderWorker(ModuleDocument &document);

    void previewWaveform(trackerboy::Note note);

    void stopPreview();

    bool isRendering();

    void quitRender();

public slots:
    void render();

private:
    ModuleDocument &mDocument;
    audio::PlaybackQueue mPb;
    trackerboy::Synth mSynth;
    trackerboy::Engine mEngine;

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
