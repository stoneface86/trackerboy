
#pragma once

#include "model/SongListModel.hpp"

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QRadioButton>

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


    QSpinBox *mRowsPerBeatSpin;
    QSpinBox *mRowsPerMeasureSpin;
    QSpinBox *mSpeedSpin;
    QSpinBox *mTempoSpin;
    QPushButton *mTempoCalcButton;
    QLineEdit *mTempoActualEdit;
    QSpinBox *mPatternSpin;
    QSpinBox *mRowsPerPatternSpin;


};
