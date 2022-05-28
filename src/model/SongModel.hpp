
#pragma once

#include "core/Module.hpp"
#include "verdigris/wobjectimpl.h"

#include <QObject>


//
// Model class for a trackerboy Song. Provides methods for safely modifying
// the current song's settings in a Module.
//
class SongModel : public QObject {

    W_OBJECT(SongModel)

public:
    explicit SongModel(Module &mod, QObject *parent = nullptr);

    int rowsPerBeat();
    int rowsPerMeasure();
    int speed();
    float tempo();
    int patternSize();

    trackerboy::Speed estimateSpeed(int tempo);

    void setRowsPerBeat(int rpb);
    void setRowsPerMeasure(int rpm);
    void setSpeed(int speed);
    void setSpeedFromTempo(int tempo);
    void setPatternSize(int rows);

//signals:

    void patternSizeChanged(int rows) W_SIGNAL(patternSizeChanged, rows)
    void speedChanged(int speed) W_SIGNAL(speedChanged, speed)
    void rowsPerBeatChanged(int rpb) W_SIGNAL(rowsPerBeatChanged, rpb)
    void rowsPerMeasureChanged(int rpm) W_SIGNAL(rowsPerMeasureChanged, rpm)
    void tempoChanged(float tempo) W_SIGNAL(tempoChanged, tempo)

    void reloaded() W_SIGNAL(reloaded)

private:
    Q_DISABLE_COPY(SongModel)

    void calcTempo(bool notify = true);

    Module &mModule;
    float mTempo;


};
