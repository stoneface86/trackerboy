
#pragma once

#include "trackerboy/Synth.hpp"
#include "trackerboy/export/Player.hpp"

#include <QThread>
#include <QMutex>

//
// Worker thread for exporting a module to a wav file
//
class WavExporter : public QThread {
    Q_OBJECT

public:
    WavExporter(
        trackerboy::Module const& mod,
        unsigned samplerate,
        QObject *parent = nullptr
    );

    void setDuration(trackerboy::Player::Duration duration);

    void setDestination(QString const& dest);

    bool failed() const;

    void cancel();

signals:
    void progressMax(int max);
    void progress(int amount);

protected:
    virtual void run() override;

private:
    QMutex mMutex;

    unsigned mSamplerate;
    trackerboy::Synth mSynth;
    trackerboy::GbApu mApu;
    trackerboy::Engine mEngine;

    trackerboy::Player::Duration mDuration;

    QString mDestination;

    bool mFailed;
    bool mAbort;

};
