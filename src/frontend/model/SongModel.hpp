
#pragma once

#include "core/Module.hpp"

#include <QObject>


//
// Model class for a trackerboy Song. Provides methods for safely modifying
// the current song's settings in a Module.
//
class SongModel : public QObject {

    Q_OBJECT

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

signals:

    void patternSizeChanged(int rows);
    void speedChanged(int speed);
    void rowsPerBeatChanged(int rpb);
    void rowsPerMeasureChanged(int rpm);
    void tempoChanged(float tempo);

    void reloaded();

private:
    Q_DISABLE_COPY(SongModel)

    void calcTempo(bool notify = true);

    Module &mModule;
    float mTempo;


};
