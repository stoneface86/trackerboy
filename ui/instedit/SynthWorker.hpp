
#pragma once

#include <QThread>
#include <QWaitCondition>
#include <QMutex>

#include <vector>

#include "Playback.hpp"
#include "trackerboy.hpp"


namespace instedit {


class SynthWorker : public QThread {

    Q_OBJECT

protected:
    void run() override;

public:
    SynthWorker(QObject *parent = nullptr);

    void setFrequency(uint16_t frequency);
    void setRuntime(trackerboy::InstrumentRuntime *runtime);
    void setLoop(bool loop);

    void stop();

private:
    Playback pb;
    trackerboy::Synth synth;
    QWaitCondition writeAvailable;
    QMutex mutex;

    // run() settings

    std::unique_ptr<float[]> buf;

    trackerboy::InstrumentRuntime *runtime; // the runtime to play
    bool loop; // reset runtime at end of program
    bool shouldStop;

};




}