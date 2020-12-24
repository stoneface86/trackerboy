
#include "core/samplerates.hpp"
#include "widgets/config/SoundConfigTab.hpp"

#include <cmath>

SoundConfigTab::SoundConfigTab(Config &config, QWidget *parent) :
    ConfigTab(Config::CategorySound, parent),
    mConfig(config),
    mLayout(),
    mDeviceGroup(tr("Device")),
    mDeviceLayout(),
    mDeviceCombo(),
    mSettingsLayout(),
    mVolumeGroup(tr("Volume")),
    mVolumeLayout(),
    mVolumeSlider(Qt::Horizontal),
    mVolumeLabel(),
    mBufferGroup(tr("Buffer size")),
    mBufferLayout(),
    mBufferSlider(Qt::Horizontal),
    mBufferLabel(),
    mSamplerateGroup(tr("Sample rate")),
    mSamplerateLayout(),
    mSamplerateCombo(),
    mLatencyGroup(tr("Latency")),
    mLatencyLayout(),
    mLatencyCheckbox(tr("Low latency playback")),
    mQualityCheckbox(tr("High quality synthesis"))
{
    // layout
    mDeviceLayout.addWidget(&mDeviceCombo);
    mDeviceGroup.setLayout(&mDeviceLayout);

    mVolumeLayout.addWidget(&mVolumeSlider);
    mVolumeLayout.addWidget(&mVolumeLabel, 0, Qt::AlignCenter);
    mVolumeGroup.setLayout(&mVolumeLayout);

    mBufferLayout.addWidget(&mBufferSlider);
    mBufferLayout.addWidget(&mBufferLabel, 0, Qt::AlignCenter);
    mBufferGroup.setLayout(&mBufferLayout);

    mSamplerateLayout.addWidget(&mSamplerateCombo);
    mSamplerateGroup.setLayout(&mSamplerateLayout);

    mLatencyLayout.addWidget(&mLatencyCheckbox);
    mLatencyGroup.setLayout(&mLatencyLayout);

    mSettingsLayout.addWidget(&mVolumeGroup, 0, 0);
    mSettingsLayout.addWidget(&mBufferGroup, 0, 1);
    mSettingsLayout.addWidget(&mSamplerateGroup, 1, 0);
    mSettingsLayout.addWidget(&mLatencyGroup, 1, 1);


    mLayout.addWidget(&mDeviceGroup);
    mLayout.addLayout(&mSettingsLayout);
    mLayout.addWidget(&mQualityCheckbox);
    mLayout.addStretch();
    setLayout(&mLayout);

    // settings
    mVolumeLabel.setAlignment(Qt::AlignCenter);
    mBufferLabel.setAlignment(Qt::AlignCenter);
    mVolumeSlider.setRange(0, 100);
    mBufferSlider.setRange(0, 10);

    // initialize slider labels
    bufferSliderChanged(0);
    volumeSliderChanged(0);

    // populate samplerate combo
    for (int i = 0; i != N_SAMPLERATES; ++i) {
        mSamplerateCombo.addItem(tr("%1 Hz").arg(SAMPLERATE_TABLE[i]));
    }

    mDeviceCombo.addItem(tr("Default device"));
    mDeviceCombo.addItems(config.mMiniaudio.deviceNames());

    connect(&mBufferSlider, &QSlider::valueChanged, this, &SoundConfigTab::bufferSliderChanged);
    connect(&mVolumeSlider, &QSlider::valueChanged, this, &SoundConfigTab::volumeSliderChanged);

    // any changes made by the user will mark this tab as "dirty"
    connect(&mSamplerateCombo, QOverload<int>::of(&QComboBox::activated), this, &SoundConfigTab::setDirty);
    connect(&mDeviceCombo, QOverload<int>::of(&QComboBox::activated), this, &SoundConfigTab::setDirty);
    connect(&mLatencyCheckbox, &QCheckBox::stateChanged, this, &SoundConfigTab::setDirty);
    connect(&mQualityCheckbox, &QCheckBox::stateChanged, this, &SoundConfigTab::setDirty);

}

void SoundConfigTab::apply(Config::Sound &soundConfig) {
    mConfig.setDevice(mDeviceCombo.currentIndex());
    mConfig.setSamplerate(mSamplerateCombo.currentIndex());
    soundConfig.buffersize = mBufferSlider.value();
    soundConfig.volume = mVolumeSlider.value();
    soundConfig.lowLatency = mLatencyCheckbox.isChecked(); 

    clean();
}

void SoundConfigTab::resetControls(Config::Sound &soundConfig) {

    mDeviceCombo.setCurrentIndex(soundConfig.deviceIndex);
    mSamplerateCombo.setCurrentIndex(soundConfig.samplerateIndex);

    mBufferSlider.setValue(soundConfig.buffersize);
    mVolumeSlider.setValue(soundConfig.volume);
    mLatencyCheckbox.setChecked(soundConfig.lowLatency);

    clean();
}

void SoundConfigTab::volumeSliderChanged(int value) {
    double db = value / 100.0;
    db *= db;
    db = 6.0 * log2(db);
    mVolumeLabel.setText(QStringLiteral("%1% (%2 dB)").arg(
        QString::number(value),
        QString::number(db, 'f', 2)
    ));
    setDirty();
}

void SoundConfigTab::bufferSliderChanged(int value) {
    mBufferLabel.setText(tr("%1 frames").arg(QString::number(value)));
    setDirty();
}
