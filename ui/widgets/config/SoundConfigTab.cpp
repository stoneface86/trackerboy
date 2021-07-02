
#include "core/audio/AudioProber.hpp"
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
    mApiLabel(tr("API")),
    mApiCombo(),
    mDeviceLabel(tr("Device")),
    mDeviceCombo(),
    mRescanLayout(),
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
    mQualityButtons(),
    mRescanning(false)
{
    // layout
    mRescanLayout.addStretch();
    mRescanLayout.addWidget(&mRescanButton);

    mDeviceLayout.addWidget(&mApiLabel,         0, 0);
    mDeviceLayout.addWidget(&mApiCombo,         0, 1);

    mDeviceLayout.addWidget(&mDeviceLabel,      1, 0);
    mDeviceLayout.addWidget(&mDeviceCombo,      1, 1);

    mDeviceLayout.addLayout(&mRescanLayout,     2, 0, 1, 2);

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

    auto &prober = AudioProber::instance();
    mApiCombo.addItems(prober.backendNames());
    mDeviceCombo.addItems(prober.deviceNames(mConfig.mSound.backendIndex));
    // populate samplerate combo
    for (int i = 0; i != N_SAMPLERATES; ++i) {
        mSamplerateCombo.addItem(tr("%1 Hz").arg(SAMPLERATE_TABLE[i]));
    }

    setupTimeSpinbox(mLatencySpin);
    mLatencySpin.setMaximum(2000);
    mLatencySpin.setValue(40);

    setupTimeSpinbox(mPeriodSpin);
    mPeriodSpin.setMaximum(500);
    mPeriodSpin.setValue(5);

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

void SoundConfigTab::apply(Config::Sound &soundConfig) {
    soundConfig.backendIndex = mApiCombo.currentIndex();
    soundConfig.deviceIndex = mDeviceCombo.currentIndex();
    soundConfig.deviceName = mDeviceCombo.currentText();
    soundConfig.samplerateIndex = mSamplerateCombo.currentIndex();

    soundConfig.latency = mLatencySpin.value();
    soundConfig.period = mPeriodSpin.value();
    soundConfig.quality = mQualityButtons.checkedId();

    clean();
}

void SoundConfigTab::resetControls(Config::Sound &soundConfig) {

    mApiCombo.setCurrentIndex(soundConfig.backendIndex);
    
    // lookup the device index again, if it doesn't exist deselect it
    auto &prober = AudioProber::instance();
    int index = prober.deviceNames(soundConfig.backendIndex).indexOf(soundConfig.deviceName);
    
    mDeviceCombo.setCurrentIndex(index);
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

void SoundConfigTab::apiChanged(int index) {
    rescan(true); // new api selected, pick the default device
    setDirty();
}

void SoundConfigTab::rescan(bool rescanDueToApiChange) {
    
    auto &prober = AudioProber::instance();

    auto const backendIndex = mApiCombo.currentIndex();
    prober.probe(backendIndex);
    // get the new list of device names
    auto const deviceNames = prober.deviceNames(backendIndex);

    int deviceIndex;
    if (rescanDueToApiChange) {
        // always select the default device
        if (backendIndex == mConfig.mSound.backendIndex) {
            // went back to configured backend, try to find the configured device
            deviceIndex = deviceNames.indexOf(mConfig.mSound.deviceName);
        } else {
            // new backend, pick the default
            deviceIndex = prober.getDefaultDevice(backendIndex);
        }
    } else {
        // try to find the new index of the previously set device
        deviceIndex = deviceNames.indexOf(mDeviceCombo.currentText());
    }
    
    if (deviceIndex == -1) {
        // not found, device possibly got disconnected, use the default
        deviceIndex = prober.getDefaultDevice(backendIndex);
        setDirty();
    }

    mRescanning = true;
    mDeviceCombo.clear();
    mDeviceCombo.addItems(deviceNames);

    mDeviceCombo.setCurrentIndex(deviceIndex);
    mRescanning = false;
}

void SoundConfigTab::onDeviceComboSelected(int index) {
    Q_UNUSED(index)

    if (!mRescanning) {
        setDirty();
    }
}
