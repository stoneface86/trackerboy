
#include "core/audio/AudioProber.hpp"
#include "core/samplerates.hpp"
#include "widgets/config/SoundConfigTab.hpp"

#include "gbapu.hpp"

#include <array>
#include <optional>
#include <cmath>

#include <QSignalBlocker>


SoundConfigTab::SoundConfigTab(QWidget *parent) :
    ConfigTab(Config::CategorySound, parent),
    mLayout(),
    mDeviceGroup(tr("Device")),
    mDeviceLayout(),
    mApiLabel(tr("API")),
    mApiCombo(),
    mDeviceLabel(tr("Device")),
    mDeviceCombo(),
    mRescanLayout(),
    mApiErrorLabel(tr("API unavailable")),
    mRescanButton(tr("Rescan")),
    mLatencyLabel(tr("Buffer size")),
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
    mPreview34(),
    mQualityButtons()
{
    // layout
    mRescanLayout.addStretch();
    mRescanLayout.addWidget(&mApiErrorLabel);
    mRescanLayout.addStretch(1);
    mRescanLayout.addWidget(&mRescanButton);
    mApiErrorLabel.setVisible(false);

    mDeviceLayout.addWidget(&mApiLabel,         0, 0);
    mDeviceLayout.addWidget(&mApiCombo,         0, 1);

    mDeviceLayout.addWidget(&mDeviceLabel,      1, 0);
    mDeviceLayout.addWidget(&mDeviceCombo,      1, 1);

    mDeviceLayout.addLayout(&mRescanLayout,     2, 1);

    mDeviceLayout.addWidget(&mLatencyLabel,     3, 0);
    mDeviceLayout.addWidget(&mLatencySpin,      3, 1);
    mDeviceLayout.addWidget(&mPeriodLabel,      4, 0);
    mDeviceLayout.addWidget(&mPeriodSpin,       4, 1);
    mDeviceLayout.addWidget(&mSamplerateLabel,  5, 0);
    mDeviceLayout.addWidget(&mSamplerateCombo,  5, 1);

    mDeviceLayout.setColumnStretch(1, 1);
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
    mDeviceCombo.setMaximumWidth(300); // device names can get pretty long sometimes

    auto &prober = AudioProber::instance();
    mApiCombo.addItems(prober.backendNames());
    // populate samplerate combo
    for (int i = 0; i != N_SAMPLERATES; ++i) {
        mSamplerateCombo.addItem(tr("%1 Hz").arg(SAMPLERATE_TABLE[i]));
    }

    setupTimeSpinbox(mLatencySpin);
    mLatencySpin.setMinimum(SoundConfig::MIN_LATENCY);
    mLatencySpin.setMaximum(SoundConfig::MAX_LATENCY);

    setupTimeSpinbox(mPeriodSpin);
    mPeriodSpin.setMinimum(SoundConfig::MIN_PERIOD);
    mPeriodSpin.setMaximum(SoundConfig::MAX_PERIOD);

    mLowQualityRadio.setToolTip(tr("Linear interpolation on all channels"));
    mMedQualityRadio.setToolTip(tr("Sinc interpolation on channels 1 and 2, linear interpolation on channels 3 and 4."));
    mHighQualityRadio.setToolTip(tr("Sinc interpolation on all channels"));
    
    mQualityButtons.addButton(&mLowQualityRadio, (int)gbapu::Apu::Quality::low);
    mQualityButtons.addButton(&mMedQualityRadio, (int)gbapu::Apu::Quality::medium);
    mQualityButtons.addButton(&mHighQualityRadio, (int)gbapu::Apu::Quality::high);

    // any changes made by the user will mark this tab as "dirty"
    connect(&mSamplerateCombo, qOverload<int>(&QComboBox::activated), this, &SoundConfigTab::setDirty);
    connect(&mApiCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &SoundConfigTab::apiChanged);
    connect(&mDeviceCombo, qOverload<int>(&QComboBox::activated), this, &SoundConfigTab::onDeviceComboSelected);
    connect(&mDeviceCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &SoundConfigTab::onDeviceComboSelected);
    connect(&mLatencySpin, qOverload<int>(&QSpinBox::valueChanged), this, &SoundConfigTab::setDirty);
    connect(&mPeriodSpin, qOverload<int>(&QSpinBox::valueChanged), this, &SoundConfigTab::setDirty);
    connect(&mQualityButtons, qOverload<QAbstractButton*, bool>(&QButtonGroup::buttonToggled), this, &SoundConfigTab::qualityRadioToggled);
    connect(&mRescanButton, &QPushButton::clicked, this, &SoundConfigTab::rescan);
}

void SoundConfigTab::setupTimeSpinbox(QSpinBox &spin) {
    spin.setSuffix(tr(" ms"));
    spin.setMinimum(1);
}

void SoundConfigTab::apply(SoundConfig &soundConfig) {
    soundConfig.setBackendIndex(mApiCombo.currentIndex());
    soundConfig.setDeviceIndex(mDeviceCombo.currentIndex());
    soundConfig.setSamplerateIndex(mSamplerateCombo.currentIndex());

    soundConfig.setLatency(mLatencySpin.value());
    soundConfig.setPeriod(mPeriodSpin.value());
    soundConfig.setQuality(mQualityButtons.checkedId());

    clean();
}

void SoundConfigTab::resetControls(SoundConfig const& soundConfig) {

    {
        QSignalBlocker blocker(&mApiCombo);
        mApiCombo.setCurrentIndex(soundConfig.backendIndex());
    }


    {
        QSignalBlocker blocker(&mDeviceCombo);
        populateDevices();
        mDeviceCombo.setCurrentIndex(soundConfig.deviceIndex());
    }

    mSamplerateCombo.setCurrentIndex(soundConfig.samplerateIndex());

    mLatencySpin.setValue(soundConfig.latency());
    mPeriodSpin.setValue(soundConfig.period());
    mQualityButtons.button(soundConfig.quality())->setChecked(true);

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

void SoundConfigTab::apiChanged(int index) {
    Q_UNUSED(index)
    rescan(true); // new api selected, pick the default device
    mApiErrorLabel.setVisible(!AudioProber::instance().backendInitialized(index));
    setDirty();
}

void SoundConfigTab::populateDevices() {
    mDeviceCombo.clear();
    mDeviceCombo.addItem(tr("Default device"));
    mDeviceCombo.addItems(AudioProber::instance().deviceNames(mApiCombo.currentIndex()));
}

void SoundConfigTab::rescan(bool rescanDueToApiChange) {
    
    auto const backendIndex = mApiCombo.currentIndex();

    auto &prober = AudioProber::instance();
    
    mDeviceCombo.blockSignals(true);

    if (rescanDueToApiChange) {
        prober.probe(backendIndex);
        populateDevices();
        mDeviceCombo.setCurrentIndex(0); // default
    } else {

        // get a copy of the current id
        // the current index becomes invalid when devices are added/removed
        // so we'll search after probing for the current device using it's id
        std::optional<ma_device_id> idCopy;
        auto id = prober.deviceId(backendIndex, mDeviceCombo.currentIndex());
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
        mDeviceCombo.setCurrentIndex(index);

    }
    mDeviceCombo.blockSignals(false);

}

void SoundConfigTab::onDeviceComboSelected(int index) {
    Q_UNUSED(index)

    //mDeviceConfig.setDevice(index);
    setDirty();
}
