
#pragma once

#include "core/Module.hpp"
#include "core/model/graph/GraphModel.hpp"

#include "trackerboy/data/Waveform.hpp"

class WaveModel : public GraphModel {

    Q_OBJECT

public:
    explicit WaveModel(QObject *parent = nullptr);

    virtual int count() override;

    virtual DataType dataAt(int i) override;

    virtual void setData(int i, DataType data) override;

    void setWaveform(Module *mod, trackerboy::Waveform *waveform);

    void setWaveformData(trackerboy::Waveform::Data const& data);

    QString waveformToString();

public slots:

    void setDataFromString(QString const &str);

    void clear();

private:
    Module *mModule;
    trackerboy::Waveform *mWaveform;

};
