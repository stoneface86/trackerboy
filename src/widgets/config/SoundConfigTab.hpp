
#pragma once

#include "widgets/config/ConfigTab.hpp"

#include <QComboBox>
#include <QSpinBox>
#include <QLabel>

//
// Tab widget for the "Sound" tab in ConfigDialog
//
class SoundConfigTab : public ConfigTab {

    Q_OBJECT

public:
    explicit SoundConfigTab(QWidget *parent = nullptr);

    void apply(SoundConfig &soundConfig);

    void resetControls(SoundConfig const& soundConfig);

private slots:

    void rescan(bool rescanDueToApiChange = false);

private:
    Q_DISABLE_COPY(SoundConfigTab)


    void populateDevices();

    void apiChanged(int index);

    QComboBox *mApiCombo;
    QComboBox *mDeviceCombo;
    QLabel *mApiErrorLabel;

    QSpinBox *mLatencySpin;
    QSpinBox *mPeriodSpin;
    QComboBox *mSamplerateCombo;


};
