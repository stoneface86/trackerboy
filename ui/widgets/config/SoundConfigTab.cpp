
#include "core/samplerates.hpp"
#include "widgets/config/SoundConfigTab.hpp"

#include "gbapu.hpp"

#include <array>
#include <cmath>


SoundConfigTab::SoundConfigTab(Config &config, QWidget *parent) :
    ConfigTab(Config::CategorySound, parent),
    mConfig(config),
    mLayout(),
    mDeviceGroup(tr("Device")),
    mDeviceLayout(),
    mDeviceCombo(),
    mBackendLayout(),
    mBackendLabel(),
    mRescanButton(tr("Rescan device list")),
    mDeviceFormLayout(),
    mLatencyLabel(tr("Latency")),
    mLatencySpin(),
    mPeriodLabel(tr("Period")),
    mPeriodSpin(),
    mSamplerateLabel(tr("Sample rate")),
    mSamplerateCombo(),
    mQualityGroup(tr("Quality")),
    mQualityLayout(),
    mQualityRadioLayout(),
    mLowQualityRadio(tr("Low")),
    mMedQualityRadio(tr("Medium")),
    mHighQualityRadio(tr("High")),
    mPreviewLayout(),
    mChannels12Label(tr("CH1 + CH2")),
    mChannels34Label(tr("CH3 + CH4")),
    mPreview12(),
    mPreview34()
{
    // layout
    mBackendLayout.addWidget(&mBackendLabel, 1);
    mBackendLayout.addWidget(&mRescanButton);

    mDeviceFormLayout.addRow(&mLatencyLabel, &mLatencySpin);
    mDeviceFormLayout.addRow(&mPeriodLabel, &mPeriodSpin);
    mDeviceFormLayout.addRow(&mSamplerateLabel, &mSamplerateCombo);

    mDeviceLayout.addWidget(&mDeviceCombo);
    mDeviceLayout.addLayout(&mBackendLayout);
    mDeviceLayout.addLayout(&mDeviceFormLayout);
    mDeviceGroup.setLayout(&mDeviceLayout);

    mQualityRadioLayout.addWidget(&mLowQualityRadio);
    mQualityRadioLayout.addWidget(&mMedQualityRadio);
    mQualityRadioLayout.addWidget(&mHighQualityRadio);
    
    mPreviewLayout.addWidget(&mChannels12Label, 0, 0);
    mPreviewLayout.addWidget(&mChannels34Label, 0, 1);
    mPreviewLayout.addWidget(&mPreview12, 1, 0);
    mPreviewLayout.addWidget(&mPreview34, 1, 1);


    mQualityLayout.addLayout(&mQualityRadioLayout);
    mQualityLayout.addLayout(&mPreviewLayout, 1);
    mQualityGroup.setLayout(&mQualityLayout);

    mLayout.addWidget(&mDeviceGroup);
    mLayout.addWidget(&mQualityGroup);
    mLayout.addStretch();
    setLayout(&mLayout);

    // settings
    mBackendLabel.setText(tr("Backend: %1").arg(mConfig.mMiniaudio.backendName()));

    mDeviceCombo.addItems(config.mMiniaudio.deviceNames());
    // populate samplerate combo
    for (int i = 0; i != N_SAMPLERATES; ++i) {
        mSamplerateCombo.addItem(tr("%1 Hz").arg(SAMPLERATE_TABLE[i]));
    }

    setupTimeSpinbox(mLatencySpin);
    mLatencySpin.setMaximum(2000.0);
    mLatencySpin.setValue(30.0);

    setupTimeSpinbox(mPeriodSpin);
    mPeriodSpin.setMaximum(500.0);
    mPeriodSpin.setValue(10.0);

    mLowQualityRadio.setToolTip(tr("Linear interpolation on all channels"));
    mMedQualityRadio.setToolTip(tr("Sinc interpolation on channels 1 and 2, linear interpolation on channels 3 and 4."));
    mHighQualityRadio.setToolTip(tr("Sinc interpolation on all channels"));
    
    mQualityButtons.addButton(&mLowQualityRadio, (int)gbapu::Apu::Quality::low);
    mQualityButtons.addButton(&mMedQualityRadio, (int)gbapu::Apu::Quality::medium);
    mQualityButtons.addButton(&mHighQualityRadio, (int)gbapu::Apu::Quality::high);

    // any changes made by the user will mark this tab as "dirty"
    connect(&mSamplerateCombo, QOverload<int>::of(&QComboBox::activated), this, &SoundConfigTab::setDirty);
    connect(&mDeviceCombo, QOverload<int>::of(&QComboBox::activated), this, &SoundConfigTab::setDirty);
    connect(&mLatencySpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SoundConfigTab::setDirty);
    connect(&mPeriodSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &SoundConfigTab::setDirty);
    connect(&mQualityButtons, qOverload<QAbstractButton*, bool>(&QButtonGroup::buttonToggled), this, &SoundConfigTab::qualityRadioToggled);
    connect(&mRescanButton, &QPushButton::clicked, this, &SoundConfigTab::rescan);
}

void SoundConfigTab::setupTimeSpinbox(QDoubleSpinBox &spin) {
    spin.setSuffix(tr(" ms"));
    spin.setDecimals(2);
    spin.setMinimum(1.0);
}

void SoundConfigTab::apply(Config::Sound &soundConfig) {
    soundConfig.deviceIndex = mDeviceCombo.currentIndex();
    soundConfig.samplerateIndex = mSamplerateCombo.currentIndex();

    soundConfig.latency = mLatencySpin.value();
    soundConfig.period = mPeriodSpin.value();
    soundConfig.quality = mQualityButtons.checkedId();

    clean();
}

void SoundConfigTab::resetControls(Config::Sound &soundConfig) {

    mDeviceCombo.setCurrentIndex(soundConfig.deviceIndex);
    mSamplerateCombo.setCurrentIndex(soundConfig.samplerateIndex);

    mLatencySpin.setValue(soundConfig.latency);
    mPeriodSpin.setValue(soundConfig.period);
    mQualityButtons.button(soundConfig.quality)->setChecked(true);

    clean();
}

void SoundConfigTab::qualityRadioToggled(QAbstractButton *btn, bool checked) {
    if (checked) {
        auto id = mQualityButtons.id(btn);
        mPreview12.setHighQuality(id != (int)gbapu::Apu::Quality::low);
        mPreview34.setHighQuality(id == (int)gbapu::Apu::Quality::high);
        setDirty();
    }
}

void SoundConfigTab::rescan() {
    auto &miniaudio = mConfig.mMiniaudio;
    
    // copy the current id for looking up
    auto deviceId = miniaudio.deviceId(mDeviceCombo.currentIndex());
    std::optional<ma_device_id> deviceIdCopy;
    if (deviceId != nullptr) {
        deviceIdCopy = *deviceId;
    }

    mDeviceCombo.clear();
    miniaudio.rescan();
    mDeviceCombo.addItems(miniaudio.deviceNames());

    int index = 0;
    if (deviceIdCopy) {
        index = miniaudio.lookupDevice(&*deviceIdCopy);
        if (index == -1) {
            // the old device wasn't found (probably got disconnected)
            // go to the default device
            index = 0;
            setDirty();
        }
    }
    mDeviceCombo.setCurrentIndex(index);
}
