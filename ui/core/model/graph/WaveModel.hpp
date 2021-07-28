
#pragma once

#include "core/model/graph/GraphModel.hpp"

#include "trackerboy/data/Waveform.hpp"

class WaveModel : public GraphModel {

    Q_OBJECT

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

public slots:

    void setDataFromString(QString const &str);

    void clear();

private:
    trackerboy::Waveform *mWaveform;

};
