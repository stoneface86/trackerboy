
#include "widgets/config/SoundConfigTab.hpp"
#include "audio/AudioEnumerator.hpp"
#include "core/StandardRates.hpp"
#include "core/misc/connectutils.hpp"

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>

#include <array>
#include <optional>
#include <cmath>



SoundConfigTab::SoundConfigTab(SoundConfig const& soundConfig, AudioEnumerator &enumerator, QWidget *parent) :
    ConfigTab(parent),
    mEnumerator(enumerator)
{

    auto layout = new QVBoxLayout;

    auto deviceGroup = new QGroupBox(tr("Device"));
    auto deviceLayout = new QGridLayout;

    // row 0
    deviceLayout->addWidget(new QLabel(tr("API")), 0, 0);
    mApiCombo = new QComboBox;
    deviceLayout->addWidget(mApiCombo, 0, 1);

    // row 1
    deviceLayout->addWidget(new QLabel(tr("Device")), 1, 0);
    mDeviceCombo = new QComboBox;
    deviceLayout->addWidget(mDeviceCombo, 1, 1);

    // row 2
    auto rescanLayout = new QHBoxLayout;
    mApiErrorLabel = new QLabel(tr("API Unavailable"));
    rescanLayout->addStretch();
    rescanLayout->addWidget(mApiErrorLabel);
    rescanLayout->addStretch(1);
    auto rescanButton = new QPushButton(tr("Rescan"));
    rescanLayout->addWidget(rescanButton);
    deviceLayout->addLayout(rescanLayout, 2, 1);

    deviceGroup->setLayout(deviceLayout);


    auto audioGroup = new QGroupBox(tr("Audio"));
    auto audioLayout = new QGridLayout;

    // row 0, buffer size (latency)
    audioLayout->addWidget(new QLabel(tr("Buffer size")), 0, 0);
    mLatencySpin = new QSpinBox;
    audioLayout->addWidget(mLatencySpin, 0, 1);

    // row 1, period
    audioLayout->addWidget(new QLabel(tr("Period")), 1, 0);
    mPeriodSpin = new QSpinBox;
    audioLayout->addWidget(mPeriodSpin, 1, 1);

    // row 2, samplerate
    audioLayout->addWidget(new QLabel(tr("Sample rate")), 2, 0);
    mSamplerateCombo = new QComboBox;
    audioLayout->addWidget(mSamplerateCombo, 2, 1);

    audioGroup->setLayout(audioLayout);

    layout->addWidget(deviceGroup);
    layout->addWidget(audioGroup);
    layout->addStretch();
    setLayout(layout);


    // settings
    mDeviceCombo->setMaximumWidth(300); // device names can get pretty long sometimes

    mApiCombo->addItems(mEnumerator.backendNames());
    // populate samplerate combo
    for (int i = 0; i != StandardRates::COUNT; ++i) {
        mSamplerateCombo->addItem(tr("%1 Hz").arg(StandardRates::get(i)));
    }

    mApiCombo->setCurrentIndex(soundConfig.backendIndex());
    populateDevices();
    mDeviceCombo->setCurrentIndex(soundConfig.deviceIndex());
    mSamplerateCombo->setCurrentIndex(soundConfig.samplerateIndex());
    mLatencySpin->setValue(soundConfig.latency());
    mPeriodSpin->setValue(soundConfig.period());

    auto setupTimeSpinbox = [](QSpinBox &spin, int min, int max) {
        spin.setSuffix(tr(" ms"));
        spin.setMinimum(min);
        spin.setMaximum(max);
    };
    setupTimeSpinbox(*mLatencySpin, SoundConfig::MIN_LATENCY, SoundConfig::MAX_LATENCY);
    setupTimeSpinbox(*mPeriodSpin, SoundConfig::MIN_PERIOD, SoundConfig::MAX_PERIOD);

    mApiErrorLabel->setVisible(false);

    // any changes made by the user will mark this tab as "dirty"
    connect(mSamplerateCombo, qOverload<int>(&QComboBox::activated), this, &SoundConfigTab::setDirty<Config::CategorySound>);
    connect(mApiCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &SoundConfigTab::apiChanged);
    connect(mDeviceCombo, qOverload<int>(&QComboBox::activated), this, &SoundConfigTab::setDirty<Config::CategorySound>);
    connect(mDeviceCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &SoundConfigTab::setDirty<Config::CategorySound>);
    connect(mLatencySpin, qOverload<int>(&QSpinBox::valueChanged), this, &SoundConfigTab::setDirty<Config::CategorySound>);
    connect(mPeriodSpin, qOverload<int>(&QSpinBox::valueChanged), this, &SoundConfigTab::setDirty<Config::CategorySound>);
    lazyconnect(rescanButton, clicked, this, rescan);
}

void SoundConfigTab::apply(SoundConfig &soundConfig) {
    soundConfig.setBackendIndex(mApiCombo->currentIndex());
    soundConfig.setDeviceIndex(mDeviceCombo->currentIndex());
    soundConfig.setSamplerateIndex(mSamplerateCombo->currentIndex());

    soundConfig.setLatency(mLatencySpin->value());
    soundConfig.setPeriod(mPeriodSpin->value());

    clean();
}

void SoundConfigTab::apiChanged(int index) {
    rescan(true); // new api selected, pick the default device
    mApiErrorLabel->setVisible(!mEnumerator.backendIsAvailable(index));
    setDirty<Config::CategorySound>();
}

void SoundConfigTab::populateDevices() {
    mDeviceCombo->clear();
    mDeviceCombo->addItems(mEnumerator.deviceNames(mApiCombo->currentIndex()));
}

void SoundConfigTab::rescan(bool rescanDueToApiChange) {
    
    auto const backendIndex = mApiCombo->currentIndex();

    QSignalBlocker blocker(mDeviceCombo);

    if (rescanDueToApiChange) {
        mEnumerator.populate(backendIndex);
        populateDevices();
        mDeviceCombo->setCurrentIndex(0); // default
    } else {

        // serialize the current device
        auto serialized = mEnumerator.serializeDevice(backendIndex, mDeviceCombo->currentIndex());
        
        // rescan
        mEnumerator.populate(backendIndex);
        populateDevices();

        // now attempt to find the saved device in the list
        // (its position may have changed or the device is no longer available)
        auto index = mEnumerator.deserializeDevice(backendIndex, serialized);
        mDeviceCombo->setCurrentIndex(index);

    }

}
