
#pragma once

#include "widgets/config/ConfigTab.hpp"
class AudioEnumerator;


class QComboBox;
class QSpinBox;
class QLabel;

//
// Tab widget for the "Sound" tab in ConfigDialog
//
class SoundConfigTab : public ConfigTab {

    Q_OBJECT

public:
    explicit SoundConfigTab(SoundConfig const& soundConfig, AudioEnumerator &audio, QWidget *parent = nullptr);

    void apply(SoundConfig &soundConfig);

private slots:

    void rescan(bool rescanDueToApiChange = false);

private:
    Q_DISABLE_COPY(SoundConfigTab)


    void populateDevices();

    void apiChanged(int index);

    AudioEnumerator &mEnumerator;

    QComboBox *mApiCombo;
    QComboBox *mDeviceCombo;
    QLabel *mApiErrorLabel;

    QSpinBox *mLatencySpin;
    QSpinBox *mPeriodSpin;
    QComboBox *mSamplerateCombo;


};
