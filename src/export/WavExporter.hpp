
#pragma once

#include "core/Module.hpp"
#include "core/ChannelOutput.hpp"
#include "verdigris/wobjectdefs.h"

#include "trackerboy/apu/DefaultApu.hpp"
#include "trackerboy/export/Player.hpp"
#include "trackerboy/Synth.hpp"

#include <QThread>
#include <QMutex>

//
// Worker thread for exporting a module to a wav file
//
class WavExporter : public QThread {
    
    W_OBJECT(WavExporter)

public:
    WavExporter(
        Module const& mod,
        int samplerate,
        QObject *parent = nullptr
    );

    void setDuration(trackerboy::Player::Duration duration);

    void setDestination(QString const& dest);

    void setChannels(ChannelOutput::Flags channels);

    void setSeparate(bool separate);

    void setSeparatePrefix(QString const& prefix);

    bool failed() const;

    void cancel();

//signals:
    void progressMax(int max) W_SIGNAL(progressMax, max)
    void progress(int amount) W_SIGNAL(progress, amount)

protected:
    virtual void run() override;

private:
    QMutex mMutex;

    int mSamplerate;
    trackerboy::DefaultApu mApu;
    trackerboy::Synth mSynth;
    trackerboy::Engine mEngine;

    trackerboy::Player::Duration mDuration;

    ChannelOutput::Flags mChannels;
    bool mSeparate;

    QString mDestination;
    QString mSeparatePrefix;

    bool mFailed;
    bool mAbort;

};
