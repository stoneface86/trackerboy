
#pragma once

#include "model/SongListModel.hpp"

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QRadioButton>

class SongPropertiesWidget : public QWidget {

    Q_OBJECT

public:
    explicit SongPropertiesWidget(SongListModel &model, QWidget *parent = nullptr);
    ~SongPropertiesWidget();


private slots:
    void onSongChanged(int index);
    void onSpeedChanged(double speed);


private:

    void calculateTempo();

    SongListModel &mModel;


    QSpinBox *mRowsPerBeatSpin;
    QSpinBox *mRowsPerMeasureSpin;
    QDoubleSpinBox *mSpeedSpin;
    QLineEdit *mTempoEdit;
    QSpinBox *mPatternSpin;
    QSpinBox *mRowsPerPatternSpin;

    bool mIgnoreSpeedChanges;

};
