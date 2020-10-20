
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
    // set the index to -1 so the first call to resetControls populates the device combobox
    backendCombo->setCurrentIndex(-1);

    connect(mUi->mBufferSizeSlider, &QSlider::valueChanged, this, &ConfigDialog::bufferSizeSliderChanged);
    connect(mUi->mVolumeSlider, &QSlider::valueChanged, this, &ConfigDialog::volumeSliderChanged);
    connect(mUi->mBassSlider, &QSlider::valueChanged, this, &ConfigDialog::bassSliderChanged);
    connect(mUi->mTrebleSlider, &QSlider::valueChanged, this, &ConfigDialog::trebleAmountSliderChanged);
    connect(mUi->mTrebleCutoffSlider, &QSlider::valueChanged, this, &ConfigDialog::trebleCutoffSliderChanged);

    connect(mUi->mBackendCombo, QOverload<int>::of(&QComboBox::activated), this, &ConfigDialog::backendActivated);
}

ConfigDialog::~ConfigDialog() {
    delete mUi;
}

void ConfigDialog::accept() {
    // update all changes to the Config object
    mConfig.setDevice(mUi->mBackendCombo->currentIndex(), mUi->mDeviceCombo->currentIndex());
    mConfig.setSamplerate(static_cast<audio::Samplerate>(mUi->mSamplerateCombo->currentIndex()));

    mConfig.setBuffersize(mUi->mBufferSizeSlider->value());
    mConfig.setVolume(mUi->mVolumeSlider->value());
    mConfig.setBassFrequency(mUi->mBassSlider->value());
    mConfig.setTreble(mUi->mTrebleSlider->value());
    mConfig.setTrebleFrequency(mUi->mTrebleCutoffSlider->value());


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

void ConfigDialog::bassSliderChanged(int value) {
    QString text("%1 Hz");
    mUi->mBassLabel->setText(text.arg(QString::number(value)));
}

void ConfigDialog::trebleAmountSliderChanged(int value) {
    QString text("%1 dB");
    mUi->mTrebleLabel->setText(text.arg(QString::number(value)));
}

void ConfigDialog::trebleCutoffSliderChanged(int value) {
    QString text("%1 Hz");
    mUi->mTrebleCutoffLabel->setText(text.arg(QString::number(value)));
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

    mUi->mSamplerateCombo->setCurrentIndex(mConfig.samplerate());

    mUi->mBufferSizeSlider->setValue(mConfig.buffersize());
    mUi->mVolumeSlider->setValue(mConfig.volume());
    mUi->mBassSlider->setValue(mConfig.bassFrequency());
    mUi->mTrebleSlider->setValue(mConfig.treble());
    mUi->mTrebleCutoffSlider->setValue(mConfig.trebleFrequency());
}



