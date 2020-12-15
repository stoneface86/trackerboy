
#pragma once

#include "widgets/config/ConfigTab.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

//
// Container struct for the "Sound" tab in ConfigDialog
//
class SoundConfigTab : public ConfigTab {

    Q_OBJECT

public:
    SoundConfigTab(Config &config, QWidget *parent = nullptr);

    void apply(Config::Sound &soundConfig);

    void resetControls(Config::Sound &soundConfig);

private slots:
    void bufferSliderChanged(int value);
    void volumeSliderChanged(int value);


private:

    Config &mConfig;

    QVBoxLayout mLayout;
        QGroupBox mDeviceGroup;
            QGridLayout mDeviceLayout;
                QComboBox mDeviceCombo;
        QGridLayout mSettingsLayout;
            QGroupBox mVolumeGroup;         // 0, 0
                QVBoxLayout mVolumeLayout;
                    QSlider mVolumeSlider;
                    QLabel mVolumeLabel;
            QGroupBox mBufferGroup;         // 0, 1
                QVBoxLayout mBufferLayout;
                    QSlider mBufferSlider;
                    QLabel mBufferLabel;
            QGroupBox mSamplerateGroup;     // 1, 0
                QGridLayout mSamplerateLayout;
                    QComboBox mSamplerateCombo;
            QGroupBox mLatencyGroup;        // 1, 1
                QGridLayout mLatencyLayout;
                    QCheckBox mLatencyCheckbox;
        QCheckBox mQualityCheckbox;


};
