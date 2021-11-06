
#include "widgets/config/SoundConfigTab.hpp"
#include "core/audio/AudioProber.hpp"
#include "core/StandardRates.hpp"

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



SoundConfigTab::SoundConfigTab(SoundConfig const& soundConfig, QWidget *parent) :
    ConfigTab(parent)
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

    auto &prober = AudioProber::instance();
    mApiCombo->addItems(prober.backendNames());
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
    mApiErrorLabel->setVisible(!AudioProber::instance().backendInitialized(index));
    setDirty<Config::CategorySound>();
}

void SoundConfigTab::populateDevices() {
    mDeviceCombo->clear();
    mDeviceCombo->addItem(tr("Default device"));
    mDeviceCombo->addItems(AudioProber::instance().deviceNames(mApiCombo->currentIndex()));
}

void SoundConfigTab::rescan(bool rescanDueToApiChange) {
    
    auto const backendIndex = mApiCombo->currentIndex();

    auto &prober = AudioProber::instance();
    
    mDeviceCombo->blockSignals(true);

    if (rescanDueToApiChange) {
        prober.probe(backendIndex);
        populateDevices();
        mDeviceCombo->setCurrentIndex(0); // default
    } else {

        // get a copy of the current id
        // the current index becomes invalid when devices are added/removed
        // so we'll search after probing for the current device using it's id
        std::optional<ma_device_id> idCopy;
        auto id = prober.deviceId(backendIndex, mDeviceCombo->currentIndex());
        if (id) {
            idCopy = *id;
        }

        // rescan
        prober.probe(backendIndex);
        populateDevices();

        int index;
        if (idCopy) {
            index = prober.indexOfDevice(backendIndex, *idCopy);
        } else {
            index = 0;
        }
        mDeviceCombo->setCurrentIndex(index);

    }
    mDeviceCombo->blockSignals(false);

}
