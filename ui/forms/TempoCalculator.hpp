
#pragma once

#include "core/model/SongModel.hpp"
#include "widgets/CustomSpinBox.hpp"
#include "widgets/SpeedLabel.hpp"
#include "widgets/TempoLabel.hpp"

#include <QDialog>
#include <QLabel>
#include <QSpinBox>


//
// Utility widget for calculating speed from a tempo.
//
class TempoCalculator : public QDialog {

    Q_OBJECT

public:

    explicit TempoCalculator(SongModel &model, QWidget *parent = nullptr);

private:

    float calculate(trackerboy::Speed speed);

    SongModel &mModel;

    TempoLabel *mTempoLabel;
    QSpinBox *mTempoSpin;
    SpeedLabel *mSpeedLabel;
    CustomSpinBox *mSpeedSpin;

};
