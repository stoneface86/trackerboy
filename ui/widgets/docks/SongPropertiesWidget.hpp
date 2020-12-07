
#pragma once

#include "model/SongListModel.hpp"

#include <QWidget>
#include <QSpinBox>
#include <QRadioButton>

class SongPropertiesWidget : public QWidget {

    Q_OBJECT

public:
    explicit SongPropertiesWidget(SongListModel &model, QWidget *parent = nullptr);
    ~SongPropertiesWidget();


private slots:
    void onSongChanged(int index);

private:

    SongListModel &mModel;


    QSpinBox *mRowsPerBeatSpin;
    QSpinBox *mRowsPerMeasureSpin;
    QRadioButton *mTempoRadio;
    QRadioButton *mSpeedRadio;

    QSpinBox *mTempoSpin;
    QDoubleSpinBox *mSpeedSpin;
    QSpinBox *mPatternSpin;
    QSpinBox *mRowsPerPatternSpin;


};
