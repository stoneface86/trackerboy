
#include "widgets/config/SoundConfigTab.hpp"
#include "audio/AudioEnumerator.hpp"
#include "core/StandardRates.hpp"
#include "midi/MidiEnumerator.hpp"
#include "utils/connectutils.hpp"

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>

//
// QGroupBox subclass containing a combobox for an API and Device
// Used for selecting a MIDI input device and Audio output device
//
class DeviceGroup : public QGroupBox {

    Q_OBJECT

public:

    explicit DeviceGroup(QString const& title) :
        QGroupBox(title)
    {
        auto layout = new QGridLayout;

        // row 0
        layout->addWidget(new QLabel(tr("API")), 0, 0);
        mApiCombo = new QComboBox;
        layout->addWidget(mApiCombo, 0, 1);

        // row 1
        layout->addWidget(new QLabel(tr("Device")), 1, 0);
        mDeviceCombo = new QComboBox;
        layout->addWidget(mDeviceCombo, 1, 1);

        // row 2
        auto rescanLayout = new QHBoxLayout;
        mErrorLabel = new QLabel(tr("API Unavailable"));
        rescanLayout->addStretch();
        rescanLayout->addWidget(mErrorLabel);
        rescanLayout->addStretch(1);
        mRescanButton = new QPushButton(tr("Rescan"));
        rescanLayout->addWidget(mRescanButton);
        layout->addLayout(rescanLayout, 2, 1);

        setLayout(layout);

        mDeviceCombo->setMaximumWidth(300); // device names can get pretty long sometimes

    }

    void setAvailable(bool available) {
        mErrorLabel->setVisible(!available);
    }

    template <class Enumerator>
    void init(Enumerator const& enumerator, int backend, int device) {
        mApiCombo->addItems(enumerator.backendNames());
        mApiCombo->setCurrentIndex(backend);
        populateDevices(enumerator);
        mDeviceCombo->setCurrentIndex(device);
        mErrorLabel->setVisible(!enumerator.backendIsAvailable(backend));
    }

    template <class Enumerator>
    void populateDevices(Enumerator const& enumerator) {
        mDeviceCombo->clear();
        mDeviceCombo->addItems(enumerator.deviceNames(mApiCombo->currentIndex()));
        mDeviceCombo->setEnabled(mDeviceCombo->count() > 0);
    }


private:
    friend class SoundConfigTab;

    QComboBox *mApiCombo;
    QComboBox *mDeviceCombo;
    QLabel *mErrorLabel;
    QPushButton *mRescanButton;

};


SoundConfigTab::SoundConfigTab(
    MidiConfig const& midiConfig,
    SoundConfig const& soundConfig,
    AudioEnumerator &audio,
    MidiEnumerator &midi,
    QWidget *parent
) :
    ConfigTab(parent),
    mAudioEnumerator(audio),
    mMidiEnumerator(midi)
{

    mAudioGroup = new DeviceGroup(tr("Output device"));

    auto audioGroup = new QGroupBox(tr("Output settings"));
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

    mMidiGroup = new DeviceGroup(tr("MIDI Input"));
    mMidiGroup->setCheckable(true);
    mMidiGroup->setChecked(midiConfig.isEnabled());

    auto layout = new QVBoxLayout;
    layout->addWidget(mAudioGroup);
    layout->addWidget(audioGroup);
    layout->addWidget(mMidiGroup);
    layout->addStretch();
    setLayout(layout);


    // settings
    mAudioGroup->init(mAudioEnumerator, soundConfig.backendIndex(), soundConfig.deviceIndex());

    // populate samplerate combo
    for (int i = 0; i != StandardRates::COUNT; ++i) {
        mSamplerateCombo->addItem(tr("%1 Hz").arg(StandardRates::get(i)));
    }

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

    mMidiGroup->init(mMidiEnumerator, midiConfig.backendIndex(), midiConfig.portIndex());

    // any changes made by the user will mark this tab as "dirty"
    connect(mSamplerateCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &SoundConfigTab::setDirty<Config::CategorySound>);
    connect(mLatencySpin, qOverload<int>(&QSpinBox::valueChanged), this, &SoundConfigTab::setDirty<Config::CategorySound>);
    connect(mPeriodSpin, qOverload<int>(&QSpinBox::valueChanged), this, &SoundConfigTab::setDirty<Config::CategorySound>);

    connect(mAudioGroup->mApiCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &SoundConfigTab::audioApiChanged);
    connect(mAudioGroup->mDeviceCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &SoundConfigTab::setDirty<Config::CategorySound>);
    lazyconnect(mAudioGroup->mRescanButton, clicked, this, audioRescan);

    lazyconnect(mMidiGroup, toggled, this, setDirty<Config::CategoryMidi>);
    connect(mMidiGroup->mApiCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &SoundConfigTab::midiApiChanged);
    connect(mMidiGroup->mDeviceCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &SoundConfigTab::setDirty<Config::CategoryMidi>);
    lazyconnect(mMidiGroup->mRescanButton, clicked, this, midiRescan);
}

void SoundConfigTab::apply(SoundConfig &soundConfig) {
    soundConfig.setBackendIndex(mAudioGroup->mApiCombo->currentIndex());
    soundConfig.setDeviceIndex(mAudioGroup->mDeviceCombo->currentIndex());
    soundConfig.setSamplerateIndex(mSamplerateCombo->currentIndex());

    soundConfig.setLatency(mLatencySpin->value());
    soundConfig.setPeriod(mPeriodSpin->value());

    clean();
}

void SoundConfigTab::apply(MidiConfig &midiConfig) {
    auto const enabled = mMidiGroup->isChecked();
    midiConfig.setEnabled(enabled);
    if (enabled) {
        midiConfig.setBackendIndex(mMidiGroup->mApiCombo->currentIndex());
        midiConfig.setPortIndex(mMidiGroup->mDeviceCombo->currentIndex());
    }

    clean();
}

template <>
void SoundConfigTab::setDirtyFromEnumerator<AudioEnumerator>() {
    setDirty<Config::CategorySound>();
}

template <>
void SoundConfigTab::setDirtyFromEnumerator<MidiEnumerator>() {
    setDirty<Config::CategoryMidi>();
}

template <class Enumerator>
void SoundConfigTab::apiChanged(Enumerator &enumerator, DeviceGroup *group, int index) {
    QSignalBlocker blocker(group->mDeviceCombo);
    enumerator.populate(index);
    group->populateDevices(enumerator);
    group->mDeviceCombo->setCurrentIndex(0); // default

    group->mErrorLabel->setVisible(!enumerator.backendIsAvailable(index));

    setDirtyFromEnumerator<Enumerator>();

}

void SoundConfigTab::audioApiChanged(int index) {
    apiChanged(mAudioEnumerator, mAudioGroup, index);
}

void SoundConfigTab::midiApiChanged(int index) {
    apiChanged(mMidiEnumerator, mMidiGroup, index);
}

template <class Enumerator>
void SoundConfigTab::rescan(Enumerator &enumerator, DeviceGroup *group) {
    QSignalBlocker blocker(group->mDeviceCombo);

    // save the current selection
    auto const current = group->mDeviceCombo->currentText();

    // rescan the device list and update the combo
    enumerator.populate(group->mApiCombo->currentIndex());
    group->populateDevices(enumerator);

    // search for the current in the new list, defaulting to 0 if not found
    auto index = group->mDeviceCombo->findText(current);
    if (index == -1) {
        // not found, select the first
        index = 0;
        setDirtyFromEnumerator<Enumerator>();
    }
    group->mDeviceCombo->setCurrentIndex(index);
}

void SoundConfigTab::audioRescan() {
    rescan(mAudioEnumerator, mAudioGroup);
}

void SoundConfigTab::midiRescan() {
    rescan(mMidiEnumerator, mMidiGroup);
}

#include "SoundConfigTab.moc"
