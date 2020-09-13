
#include "ConfigDialog.hpp"

#include "portaudio.h"

#include <algorithm>
#include <cmath>

static const char *SAMPLING_RATE_STR[] = {
    "11,025 Hz",
    "22,050 Hz",
    "44,100 Hz",
    "48,000 Hz",
    "96,000 Hz"
};


ConfigDialog::ConfigDialog(Config &config, QWidget *parent) :
    mConfig(config),
    mDeviceManager(),
    mIgnoreSelections(false),
    QDialog(parent)
{
    setupUi(this);

    // populate the host combo with all available host apis
    // we only need to do this once
    auto &deviceTable = audio::DeviceTable::instance();
    auto &hosts = deviceTable.hosts();
    for (auto &host : hosts) {
        mHostApiCombo->addItem(QString::fromLatin1(host.info->name));
    }

    connect(mBufferSizeSlider, &QSlider::valueChanged, this, &ConfigDialog::bufferSizeSliderChanged);
    connect(mVolumeSlider, &QSlider::valueChanged, this, &ConfigDialog::volumeSliderChanged);

    connect(mGainSlider1, &QSlider::valueChanged, this, [this](int value) { gainChanged(0, value); });
    connect(mGainSlider2, &QSlider::valueChanged, this, [this](int value) { gainChanged(1, value); });
    connect(mGainSlider3, &QSlider::valueChanged, this, [this](int value) { gainChanged(2, value); });
    connect(mGainSlider4, &QSlider::valueChanged, this, [this](int value) { gainChanged(3, value); });

    connect(mHostApiCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConfigDialog::hostApiSelected);
    connect(mDeviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConfigDialog::deviceSelected);
    connect(mSamplerateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConfigDialog::samplerateSelected);

    // reset all controls to our Config's settings
    resetControls();

}

void ConfigDialog::accept() {
    // update all changes to the Config object
    mConfig.setDeviceId(mDeviceManager.portaudioDevice());
    mConfig.setSamplerate(mDeviceManager.samplerates()[mDeviceManager.currentSamplerate()]);
    mConfig.setBuffersize(mBufferSizeSlider->value());
    mConfig.setVolume(mVolumeSlider->value());
    mConfig.setGain(trackerboy::ChType::ch1, mGainSlider1->value());
    mConfig.setGain(trackerboy::ChType::ch2, mGainSlider2->value());
    mConfig.setGain(trackerboy::ChType::ch3, mGainSlider3->value());
    mConfig.setGain(trackerboy::ChType::ch4, mGainSlider4->value());


    QDialog::accept();
}

void ConfigDialog::reject() {
    // reset all settings
    resetControls();

    QDialog::reject();
}


void ConfigDialog::bufferSizeSliderChanged(int value) {
    QString text("%1 ms");
    mBufferSizeLabel->setText(text.arg(QString::number(value)));
}

void ConfigDialog::volumeSliderChanged(int value) {
    QString text("%1%");
    mVolumeLabel->setText(text.arg(QString::number(value)));
}

void ConfigDialog::hostApiSelected(int index) {
    if (!mIgnoreSelections) {
        mDeviceManager.setCurrentApi(index);

        // populate the device combo box with devices for this host
        mIgnoreSelections = true;
        fillDeviceCombo(index);
        mIgnoreSelections = false;

        mDeviceCombo->setCurrentIndex(mDeviceManager.currentDevice());
    }
}

void ConfigDialog::deviceSelected(int index) {
    
    // ignore this signal when items are being added to the combobox
    if (!mIgnoreSelections) {
        mIgnoreSelections = true;
        mDeviceManager.setCurrentDevice(index);
        fillSamplerateCombo();
        mSamplerateCombo->setCurrentIndex(mDeviceManager.currentSamplerate());
        mIgnoreSelections = false;
    }
}

void ConfigDialog::samplerateSelected(int index) {
    if (!mIgnoreSelections) {
        mDeviceManager.setCurrentSamplerate(index);
    }
}

void ConfigDialog::gainChanged(int channel, int value) {
    QLabel *label;
    switch (channel) {
        case 0:
            label = mGainLabel1;
            break;
        case 1:
            label = mGainLabel2;
            break;
        case 2:
            label = mGainLabel3;
            break;
        default:
            label = mGainLabel4;
            break;
    }
    label->setText(QString::asprintf("%+d.%d dB", value / 10, abs(value) % 10));
}

void ConfigDialog::fillDeviceCombo(int hostIndex) {
    mDeviceCombo->clear();

    auto &deviceTable = audio::DeviceTable::instance();
    auto devicesBegin = deviceTable.devicesBegin(hostIndex);
    auto devicesEnd = deviceTable.devicesEnd(hostIndex);
    for (auto iter = devicesBegin; iter != devicesEnd; ++iter) {
        mDeviceCombo->addItem(QString::fromLatin1(iter->info->name));
    }
}

void ConfigDialog::fillSamplerateCombo() {
    mSamplerateCombo->clear();
    auto &samplerates = mDeviceManager.samplerates();
    for (auto rate : samplerates) {
        mSamplerateCombo->addItem(QString::fromLatin1(SAMPLING_RATE_STR[rate]));
    }
}


void ConfigDialog::resetControls() {
    auto &deviceTable = audio::DeviceTable::instance();

    mIgnoreSelections = true;
    mDeviceManager.setPortaudioDevice(mConfig.deviceId());

    int host = mDeviceManager.currentHost();
    mHostApiCombo->setCurrentIndex(host);
    fillDeviceCombo(host);

    mDeviceCombo->setCurrentIndex(mDeviceManager.currentDevice());

    // reset samplerate
    fillSamplerateCombo();
    int samplerate = mConfig.samplerate();
    int samplerateIndex = 0;
    for (auto rate : mDeviceManager.samplerates()) {
        if (rate >= samplerate) {
            break;
        }
        ++samplerateIndex;
    }

    mDeviceManager.setCurrentSamplerate(samplerateIndex);
    mSamplerateCombo->setCurrentIndex(samplerateIndex);

    mIgnoreSelections = false;

    mBufferSizeSlider->setValue(mConfig.buffersize());
    mVolumeSlider->setValue(mConfig.volume());
    mGainSlider1->setValue(mConfig.gain(trackerboy::ChType::ch1));
    mGainSlider2->setValue(mConfig.gain(trackerboy::ChType::ch2));
    mGainSlider3->setValue(mConfig.gain(trackerboy::ChType::ch3));
    mGainSlider4->setValue(mConfig.gain(trackerboy::ChType::ch4));


}



