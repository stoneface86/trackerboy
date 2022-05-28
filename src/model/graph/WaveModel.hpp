
#pragma once

#include "model/graph/GraphModel.hpp"
#include "verdigris/wobjectimpl.h"

#include "trackerboy/data/Waveform.hpp"

#include <QString>

class WaveModel : public GraphModel {

    W_OBJECT(WaveModel)

public:
    explicit WaveModel(Module &mod, QObject *parent = nullptr);

    virtual int count() override;

    virtual DataType dataAt(int i) override;

    virtual void setData(int i, DataType data) override;

    //
    // Set the waveform that will be visible/editable by this model.
    // Note that caller assumes responsiblilty for the lifetime of the given
    // waveform.
    //
    void setWaveform(trackerboy::Waveform *waveform);

    void setWaveformData(trackerboy::Waveform::Data const& data);

    QString waveformToString();

    void setDataFromString(QString const &str);

    void clear();

private:
    trackerboy::Waveform *mWaveform;

};
