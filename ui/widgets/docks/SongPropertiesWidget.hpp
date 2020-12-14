
#pragma once

#include "model/SongListModel.hpp"

#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>


//
// Composite widget for the "Song Properties" dock. Contains settings for the
// current song being edited. User can set the tempo, number of patterns and
// size of a pattern.
//
class SongPropertiesWidget : public QWidget {

    Q_OBJECT

public:
    explicit SongPropertiesWidget(SongListModel &model, QWidget *parent = nullptr);
    ~SongPropertiesWidget();


private slots:
    void onSongChanged(int index);
    void calculateTempo();
    void calculateActualTempo(int value = 0);

    void updatePatternSpin(const QModelIndex &parent, int first, int last);

private:
    

    SongListModel &mModel;

    QFormLayout mLayout;
        QSpinBox mRowsPerBeatSpin;
        QSpinBox mRowsPerMeasureSpin;
        QGridLayout mLayoutSpeed;
            QSpinBox mSpeedSpin;            // 0, 0
            QLineEdit mTempoActualEdit;     // 0, 1
            QSpinBox mTempoSpin;            // 1, 0
            QPushButton mTempoCalcButton;   // 1, 1    
        QSpinBox mPatternSpin;
        QSpinBox mRowsPerPatternSpin;


};
