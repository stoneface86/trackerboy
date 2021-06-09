
#pragma once

#include "core/model/graph/GraphModel.hpp"

class ModuleDocument;

#include "trackerboy/data/Waveform.hpp"

class WaveModel : public GraphModel {

public:
    explicit WaveModel(QObject *parent = nullptr);

    virtual int count() override;

    virtual DataType dataAt(int i) override;

    virtual void setData(int i, DataType data) override;

    void setWaveform(ModuleDocument *doc, int waveIndex);

    void setWaveformData(trackerboy::Waveform::Data const& data);

    QString waveformToString();

public slots:

    void setDataFromString(QString const &str);

    void clear();

private:
    ModuleDocument *mDocument;
    trackerboy::Waveform *mWaveform;

};
