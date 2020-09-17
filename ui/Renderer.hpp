
#pragma once

#include <QObject>
#include <QThread>

#include "RenderWorker.hpp"


//
// Renderer class handles pattern playback and instrument/waveform previews
//
class Renderer : public QObject {

    Q_OBJECT

public:

    Renderer(ModuleDocument &document, QObject *parent = nullptr);
    ~Renderer();

    RenderWorker* worker();

public slots:
    void previewWaveform(trackerboy::Note note);

    void previewInstrument(trackerboy::Note note);

    void stopPreview();


signals:
    void playing(); // emitted when music starts playing
    void stopped(); // emitted when music stops playing via halt effect or by user action

    void rendering(); // emitted when the render work is started

private:

    void render();

    RenderWorker *mWorker;
    QThread mThread;

    bool mRendering;
};
