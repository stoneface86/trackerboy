
#include "ConfigDialog.hpp"

#include <QMessageBox>

#include <algorithm>
#include <cmath>

#pragma warning(push, 0)
#include "ui_ConfigDialog.h"
#pragma warning(pop)


ConfigDialog::ConfigDialog(audio::BackendTable &backendTable, Config &config, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::ConfigDialog()),
    mBackendTable(backendTable),
    mConfig(config),
    mLastBackendIndex(-1)
{
    mUi->setupUi(this);

    // populate the backends combobox with all available backends
    // we only need to do this once
    //auto &deviceConfig = mConfig.deviceConfig();
    //auto soundio = deviceConfig.soundio();
    auto backendCombo = mUi->mBackendCombo;
    unsigned backendCount = mBackendTable.size();
    for (unsigned i = 0; i < backendCount; ++i) {
        backendCombo->addItem(QString::fromLatin1(mBackendTable.name(i)));
    }

    connect(mUi->mBufferSizeSlider, &QSlider::valueChanged, this, &ConfigDialog::bufferSizeSliderChanged);
    connect(mUi->mVolumeSlider, &QSlider::valueChanged, this, &ConfigDialog::volumeSliderChanged);

    connect(mUi->mGainSlider1, &QSlider::valueChanged, this, [this](int value) { gainChanged(0, value); });
    connect(mUi->mGainSlider2, &QSlider::valueChanged, this, [this](int value) { gainChanged(1, value); });
    connect(mUi->mGainSlider3, &QSlider::valueChanged, this, [this](int value) { gainChanged(2, value); });
    connect(mUi->mGainSlider4, &QSlider::valueChanged, this, [this](int value) { gainChanged(3, value); });

    connect(mUi->mBackendCombo, QOverload<int>::of(&QComboBox::activated), this, &ConfigDialog::backendActivated);
}

ConfigDialog::~ConfigDialog() {
    delete mUi;
}

void ConfigDialog::accept() {
    // update all changes to the Config object
    mConfig.setDevice(mUi->mBackendCombo->currentIndex(), mUi->mDeviceCombo->currentIndex());

    mConfig.setBuffersize(mUi->mBufferSizeSlider->value());
    mConfig.setVolume(mUi->mVolumeSlider->value());
    mConfig.setGain(trackerboy::ChType::ch1, mUi->mGainSlider1->value());
    mConfig.setGain(trackerboy::ChType::ch2, mUi->mGainSlider2->value());
    mConfig.setGain(trackerboy::ChType::ch3, mUi->mGainSlider3->value());
    mConfig.setGain(trackerboy::ChType::ch4, mUi->mGainSlider4->value());


    QDialog::accept();
}

void ConfigDialog::reject() {
    // reset all settings
    resetControls();

    QDialog::reject();
}

void ConfigDialog::showEvent(QShowEvent *evt) {
    mUi->mTabWidget->setCurrentIndex(0);
    QDialog::showEvent(evt);
}

void ConfigDialog::bufferSizeSliderChanged(int value) {
    QString text("%1 ms");
    mUi->mBufferSizeLabel->setText(text.arg(QString::number(value)));
}

void ConfigDialog::volumeSliderChanged(int value) {
    QString text("%1%");
    mUi->mVolumeLabel->setText(text.arg(QString::number(value)));
}

void ConfigDialog::backendActivated(int index) {
    if (index != mLastBackendIndex) {


        int err = 0;
        if (!mBackendTable.isConnected(index)) {
            err = mBackendTable.reconnect(index);
        }


        if (err) {
            QMessageBox::warning(
                        this,
                        "Trackerboy",
                        "Could not connect to backend");

            mUi->mBackendCombo->setCurrentIndex(mLastBackendIndex);
            return;
        }
        mBackendTable.rescan(index);

        auto &backend = mBackendTable[index];
        auto soundio = backend.soundio;
        auto &table = backend.table;

        if (table.isEmpty()) {
            QMessageBox::warning(this, "Trackerboy", "This backend has no available output devices");
            mUi->mBackendCombo->setCurrentIndex(mLastBackendIndex);
            return;
        }

        int i = 0;
        int const deviceDefault = table.defaultDevice();
        int deviceIndex = 0;
        auto deviceCombo = mUi->mDeviceCombo;
        deviceCombo->clear();
        for (auto device : table) {
            auto soundioDev = soundio_get_output_device(soundio, device.deviceId);
            if (device.deviceId == deviceDefault) {
                deviceIndex = i;
            }
            QString name = QString::fromLatin1(soundioDev->name);
            deviceCombo->addItem(name);
            soundio_device_unref(soundioDev);
            ++i;
        }
        deviceCombo->setCurrentIndex(deviceIndex);

        mLastBackendIndex = index;
    }
}


void ConfigDialog::gainChanged(int channel, int value) {
    QLabel *gainLabel;
    switch (channel) {
        case 0:
            gainLabel = mUi->mGainLabel1;
            break;
        case 1:
            gainLabel = mUi->mGainLabel2;
            break;
        case 2:
            gainLabel = mUi->mGainLabel3;
            break;
        default:
            gainLabel = mUi->mGainLabel4;
            break;
    }
    QString text = QString("%1%2.%3 dB")
                    .arg(value < 0 ? '-' : '+')
                    .arg(value / 10)
                    .arg(abs(value) % 10);
    gainLabel->setText(text);
}


void ConfigDialog::resetControls() {

    // Sound tab

    auto backendCombo = mUi->mBackendCombo;
    auto deviceCombo = mUi->mDeviceCombo;

    int backendIndex = mConfig.backendIndex();
    if (backendCombo->currentIndex() != backendIndex) {
        // backend index does not match config, reset it
        backendCombo->setCurrentIndex(backendIndex);
        mLastBackendIndex = backendIndex;
        deviceCombo->clear();
        auto &backend = mBackendTable[backendIndex];
        auto &deviceTable = backend.table;
        auto soundio = backend.soundio;
        for (auto device : deviceTable) {
            auto soundioDev = soundio_get_output_device(soundio, device.deviceId);
            QString name = QString::fromLatin1(soundioDev->name);
            deviceCombo->addItem(name);
            soundio_device_unref(soundioDev);
        }
    }
    deviceCombo->setCurrentIndex(mConfig.deviceIndex());

    mUi->mBufferSizeSlider->setValue(mConfig.buffersize());
    mUi->mVolumeSlider->setValue(mConfig.volume());

    // Mixer tab
    mUi->mGainSlider1->setValue(mConfig.gain(trackerboy::ChType::ch1));
    mUi->mGainSlider2->setValue(mConfig.gain(trackerboy::ChType::ch2));
    mUi->mGainSlider3->setValue(mConfig.gain(trackerboy::ChType::ch3));
    mUi->mGainSlider4->setValue(mConfig.gain(trackerboy::ChType::ch4));


}



