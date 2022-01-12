
#pragma once

#include "model/SongModel.hpp"
#include "widgets/CustomSpinBox.hpp"

#include <QDialog>
#include <QSpinBox>

class QLabel;
class SpinBox;


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

    QLabel *mTempoLabel;
    QSpinBox *mTempoSpin;
    QLabel *mSpeedLabel;
    CustomSpinBox *mSpeedSpin;

};
