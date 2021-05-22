
#pragma once

#include "widgets/config/ConfigTab.hpp"
#include "widgets/config/SoundQualityPreview.hpp"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

//
// Tab widget for the "Sound" tab in ConfigDialog
//
class SoundConfigTab : public ConfigTab {

    Q_OBJECT

public:
    SoundConfigTab(Config &config, QWidget *parent = nullptr);

    void apply(Config::Sound &soundConfig);

    void resetControls(Config::Sound &soundConfig);

private slots:

    void qualityRadioToggled(QAbstractButton *btn, bool checked);

    void rescan();

private:
    Q_DISABLE_COPY(SoundConfigTab)

    void setupTimeSpinbox(QSpinBox &spin);

    Config &mConfig;

    QVBoxLayout mLayout;
        QGroupBox mDeviceGroup;
            QVBoxLayout mDeviceLayout;
                QComboBox mDeviceCombo;
                QHBoxLayout mBackendLayout;
                    QLabel mBackendLabel;
                    QPushButton mRescanButton;
                QFormLayout mDeviceFormLayout;
                    QLabel mLatencyLabel;
                    QSpinBox mLatencySpin;
                    QLabel mPeriodLabel;
                    QSpinBox mPeriodSpin;
                    QLabel mSamplerateLabel;
                    QComboBox mSamplerateCombo;
        QGroupBox mQualityGroup;
            QVBoxLayout mQualityLayout;
                QHBoxLayout mQualityRadioLayout;
                    QRadioButton mLowQualityRadio;
                    QRadioButton mMedQualityRadio;
                    QRadioButton mHighQualityRadio;
                QGridLayout mPreviewLayout;
                    QLabel mChannels12Label;        // 0, 0
                    QLabel mChannels34Label;        // 0, 1
                    SoundQualityPreview mPreview12; // 1, 0
                    SoundQualityPreview mPreview34; // 1, 1

    QButtonGroup mQualityButtons;

};
