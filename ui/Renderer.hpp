
#pragma once

#include "model/ModuleDocument.hpp"
#include "model/InstrumentListModel.hpp"
#include "model/WaveListModel.hpp"
#include "Config.hpp"
#include "RendererThread.hpp"

#include "trackerboy/note.hpp"

#include "miniaudio.h"

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QWaitCondition>

#include <cstdint>

//
// Renderer class handles pattern playback and instrument/waveform previews
//
class Renderer : public QObject {

    Q_OBJECT

public:

    Renderer(ModuleDocument &document,
             InstrumentListModel &instrumentModel,
             WaveListModel &waveModel,
             QObject *parent = nullptr);
    ~Renderer();

    void setConfig(Config::Sound const &config);

public slots:
    void play();
    void playPattern();
    void playFromStart();
    void playFromCursor();

    void previewWaveform(trackerboy::Note note);

    void previewInstrument(trackerboy::Note note);

    void stopPreview();

    void stopMusic();

    void stop();

signals:
    void playing(); // emitted when music starts playing
    void stopped(); // emitted when music stops playing via halt effect or by user action

private:

    QWaitCondition mAudioStopCondition;
    RendererThread mRendererThread;
    
    QMutex mMutex;
    bool mRunning;
    bool mStopBackground;

    
    QThread *mBackgroundThread; // thread to stop callback thread when signaled

    void handleBackground();
    static void backgroundThreadRun(Renderer *renderer);

};
