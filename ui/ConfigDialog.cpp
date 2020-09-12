
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


ConfigDialog::ConfigDialog(QWidget *parent) :
    mDeviceManager(),
    mSamplerateVec(),
    mLastSamplerate(audio::Samplerate::s44100),
    mIgnoreSamplerateSelection(false),
    QDialog(parent)
{
    setupUi(this);

    connect(mBufferSizeSlider, &QSlider::valueChanged, this, &ConfigDialog::bufferSizeSliderChanged);
    connect(mVolumeSlider, &QSlider::valueChanged, this, &ConfigDialog::volumeSliderChanged);

    connect(mGainSlider1, &QSlider::valueChanged, this, [this](int value) { gainChanged(0, value); });
    connect(mGainSlider2, &QSlider::valueChanged, this, [this](int value) { gainChanged(1, value); });
    connect(mGainSlider3, &QSlider::valueChanged, this, [this](int value) { gainChanged(2, value); });
    connect(mGainSlider4, &QSlider::valueChanged, this, [this](int value) { gainChanged(3, value); });

    connect(mHostApiCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConfigDialog::hostApiSelected);
    connect(mDeviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConfigDialog::deviceSelected);
    connect(mSamplerateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConfigDialog::samplerateSelected);


    auto &hosts = mDeviceManager.hosts();
    for (auto host : hosts) {
        mHostApiCombo->addItem(QString::fromLatin1(host->name));
    }
    mHostApiCombo->setCurrentIndex(mDeviceManager.currentHost());

}

void ConfigDialog::accept() {
    // update all changes to the Config object
    QDialog::accept();
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
    mDeviceManager.setCurrentApi(index);
    mDeviceCombo->clear();
    auto &devices = mDeviceManager.devices();
    for (auto dev : devices) {
        mDeviceCombo->addItem(QString::fromLatin1(dev.mInfo->name));
    }
    mDeviceCombo->setCurrentIndex(mDeviceManager.currentDevice());

}

void ConfigDialog::deviceSelected(int index) {
    if (index == -1) {
        return;
    }

    // update the sampling rate combo with this device's supported sampling rates
    mIgnoreSamplerateSelection = true;

    int curSamplerateIndex = mSamplerateCombo->currentIndex();
    mSamplerateCombo->clear();
    mSamplerateVec.clear();

    auto &dev = mDeviceManager.devices()[index];
    for (int i = 0; i != 5; ++i) {
        int samplerateFlag = 1 << i;
        if (!!(dev.mSamplerates & samplerateFlag)) {
            mSamplerateCombo->addItem(QString::fromLatin1(SAMPLING_RATE_STR[i]));
            mSamplerateVec.push_back(static_cast<audio::Samplerate>(samplerateFlag));
        }
    }

    mIgnoreSamplerateSelection = false;

    if (mSamplerateVec.size() == 1) {
        mSamplerateCombo->setCurrentIndex(0); // no need to search
    } else {
        // search for the last set sampling rate
        int rateIndex = 0;
        for (auto rate : mSamplerateVec) {
            if (static_cast<int>(rate) == static_cast<int>(mLastSamplerate)) {
                break;
            } else if (static_cast<int>(rate) > static_cast<int>(mLastSamplerate)) {
                // could not find it, use the closest one
                --rateIndex;
                break;
            }
            ++rateIndex;
        }
        mSamplerateCombo->setCurrentIndex(rateIndex);
    }


    //mSamplerateCombo->setCurrentIndex(std::min(curSamplerateIndex, mSamplerateCombo->count() - 1));
}

void ConfigDialog::samplerateSelected(int index) {
    if (!mIgnoreSamplerateSelection) {
        mLastSamplerate = mSamplerateVec[index];
    }
}

void ConfigDialog::gainChanged(int channel, int value) {
    int integral = value / 10;
    int fractional = abs(value) % 10;
    QString text; // = QString("%1.%2 dB").arg(QString::number(value / 10), QString::number(value % 10));
    text.sprintf("%+d.%d dB", integral, fractional);
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
    label->setText(text);
}
