
#pragma once

#include "core/Module.hpp"

#include <QObject>


//
// Model class for a trackerboy Song. Provides methods for safely modifying
// the song's properties for a ModuleDocument.
//
class SongModel : public QObject {

    Q_OBJECT

public:
    explicit SongModel(Module &mod, QObject *parent = nullptr);

    void reload();

    int rowsPerBeat();
    int rowsPerMeasure();
    int speed();
    int patternSize();

signals:

    void patternSizeChanged(int rows);
    void speedChanged(int speed);
    void rowsPerBeatChanged(int rpb);
    void rowsPerMeasureChanged(int rpm);

public slots:

    void setRowsPerBeat(int rpb);
    void setRowsPerMeasure(int rpm);
    void setSpeed(int speed);
    void setPatternSize(int rows);

private:

    Module &mModule;


};
