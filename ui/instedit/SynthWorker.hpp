
#pragma once

#include <QObject>
#include <QTimer>
#include <QWaitCondition>
#include <QMutex>

#include <vector>

#include "audio.hpp"
#include "trackerboy/instrument/InstrumentRuntime.hpp"
#include "trackerboy/note.hpp"


namespace instedit {


class SynthWorker : public QObject {

    Q_OBJECT

public:
    SynthWorker(QObject *parent = nullptr);
    ~SynthWorker();

    void setRuntime(trackerboy::InstrumentRuntime *runtime);
    
public slots:
    void play(trackerboy::Note note);
    void stop();

private slots:
    void onTimeout();

private:
    audio::PlaybackQueue pb;
    trackerboy::Synth synth;
    QTimer *timer;
    QMutex synthMutex;

    // run() settings

    std::unique_ptr<int16_t[]> buf;

    trackerboy::InstrumentRuntime *runtime; // the runtime to play
    trackerboy::WaveTable wtable;
    bool playing; // true if a note is being played
    bool frameOut; // true if we are in process of outputting a frame to the queue
    size_t frameOffset; // offset in the frame buffer to start outputting from

};




}
