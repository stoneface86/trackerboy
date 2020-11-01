
#include "ConfigDialog.hpp"
#include "samplerates.hpp"

#include "trackerboy/synth/Synth.hpp"

#include <QMessageBox>
#include <QPushButton>

#include <algorithm>
#include <cmath>

#pragma warning(push, 0)
#include "ui_ConfigDialog.h"
#pragma warning(pop)


ConfigDialog::ConfigDialog(Config &config, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::ConfigDialog()),
    mConfig(config),
    mDirtyFlags(0)
{
    mUi->setupUi(this);
    auto applyButton = mUi->buttonBox->button(QDialogButtonBox::Apply);
    applyButton->setEnabled(false);
    connect(applyButton, &QPushButton::clicked, this, &ConfigDialog::apply);

    // populate device combo
    config.getDevices();
    auto deviceCombo = mUi->mDeviceCombo;
    deviceCombo->addItem("Default device");
    for (unsigned i = 0; i != config.mDeviceCount; ++i) {
        deviceCombo->addItem(QString::fromLatin1(config.mDeviceList[i].name));
    }

    // populate samplerate combo
    auto samplerateCombo = mUi->mSamplerateCombo;
    for (int i = 0; i != N_SAMPLERATES; ++i) {
        samplerateCombo->addItem(QString("%1 Hz").arg(SAMPLERATE_TABLE[i]));
    }

    connect(mUi->buffersizeSlider, &QSlider::valueChanged, this, &ConfigDialog::bufferSizeSliderChanged);
    connect(mUi->mVolumeSlider, &QSlider::valueChanged, this, &ConfigDialog::volumeSliderChanged);

    connect(mUi->mSamplerateCombo, QOverload<int>::of(&QComboBox::activated), this, &ConfigDialog::samplerateActivated);
    connect(mUi->mDeviceCombo, QOverload<int>::of(&QComboBox::activated), this, &ConfigDialog::samplerateActivated);
}

ConfigDialog::~ConfigDialog() {
    delete mUi;
}

void ConfigDialog::accept() {
    apply();
    QDialog::accept();
}

void ConfigDialog::reject() {
    // reset all settings
    resetControls();

    QDialog::reject();
}

void ConfigDialog::apply() {
    // update all changes to the Config object

    if (!!(mDirtyFlags & DIRTY_FLAG_SOUND)) {
        auto &soundConfig = mConfig.mSound;
        mConfig.setDevice(mUi->mDeviceCombo->currentIndex());
        mConfig.setSamplerate(mUi->mSamplerateCombo->currentIndex());
        soundConfig.buffersize = mUi->buffersizeSlider->value();
        soundConfig.volume = mUi->mVolumeSlider->value();
        soundConfig.lowLatency = mUi->lowLatencyCheckbox->isChecked();

        mConfig.applySound();
    }

    clean();
}

void ConfigDialog::showEvent(QShowEvent *evt) {
    mUi->mTabWidget->setCurrentIndex(0);
    QDialog::showEvent(evt);
}

void ConfigDialog::bufferSizeSliderChanged(int value) {
    QString text("%1 frames");
    mUi->buffersizeLabel->setText(text.arg(QString::number(value)));
    setDirty(DIRTY_FLAG_SOUND);
}

void ConfigDialog::volumeSliderChanged(int value) {
    QString text("%1% (%2 dB)");
    double db = value / 100.0;
    db *= db;
    db = 6.0 * log2(trackerboy::Synth::HEADROOM * db);
    mUi->mVolumeLabel->setText(text.arg(
        QString::number(value),
        QString::number(db, 'f', 2)
        ));
    setDirty(DIRTY_FLAG_SOUND);
}

void ConfigDialog::samplerateActivated(int index) {
    setDirty(DIRTY_FLAG_SOUND);
}


void ConfigDialog::resetControls() {

    // Sound tab
    auto &soundConfig = mConfig.mSound;

    mUi->mDeviceCombo->setCurrentIndex(soundConfig.deviceIndex);
    mUi->mSamplerateCombo->setCurrentIndex(soundConfig.samplerateIndex);

    mUi->buffersizeSlider->setValue(soundConfig.buffersize);
    mUi->mVolumeSlider->setValue(soundConfig.volume);
    mUi->lowLatencyCheckbox->setChecked(soundConfig.lowLatency);

    clean();
}

void ConfigDialog::setDirty(int flag) {
    if (!mDirtyFlags) {
        mUi->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
    }
    mDirtyFlags |= flag;
}

void ConfigDialog::clean() {
    mDirtyFlags = 0;
    mUi->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
}


