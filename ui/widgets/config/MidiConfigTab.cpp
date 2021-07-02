
#include "widgets/config/MidiConfigTab.hpp"

#include "core/midi/MidiProber.hpp"

MidiConfigTab::MidiConfigTab(QWidget *parent) :
    ConfigTab(Config::CategoryMidi, parent),
    mRescanning(false),
    mLayout(),
    mMidiGroup(tr("MIDI input")),
    mMidiLayout(),
    mApiLabel(tr("API")),
    mApiCombo(),
    mPortLabel(tr("Device")),
    mPortCombo(),
    mRescanLayout(),
    mRescanButton(tr("Rescan"))
{

    mRescanLayout.addStretch();
    mRescanLayout.addWidget(&mRescanButton);

    mMidiLayout.addWidget(&mApiLabel, 0, 0);
    mMidiLayout.addWidget(&mApiCombo, 0, 1);
    mMidiLayout.addWidget(&mPortLabel, 1, 0);
    mMidiLayout.addWidget(&mPortCombo, 1, 1);
    mMidiLayout.addLayout(&mRescanLayout, 2, 0, 1, 2);
    mMidiLayout.setColumnStretch(1, 1);
    mMidiGroup.setLayout(&mMidiLayout);
    mMidiGroup.setCheckable(true);

    mLayout.addWidget(&mMidiGroup);
    mLayout.addStretch();
    setLayout(&mLayout);


    auto &prober = MidiProber::instance();
    mApiCombo.addItems(prober.backendNames());
    mApiCombo.setCurrentIndex(-1);

    connect(&mApiCombo, qOverload<int>(&QComboBox::currentIndexChanged), this,
        [this](int index) {
            auto &prober = MidiProber::instance();
            prober.setBackend(index);
            mPortCombo.clear();
            mPortCombo.addItems(prober.portNames());
            mPortCombo.setEnabled(mPortCombo.count() > 0);

            setDirty();
        });

    connect(&mApiCombo, qOverload<int>(&QComboBox::activated), this, &MidiConfigTab::setDirty);
    connect(&mPortCombo, qOverload<int>(&QComboBox::activated), this, &MidiConfigTab::setDirty);
    connect(&mMidiGroup, &QGroupBox::toggled, this, &MidiConfigTab::setDirty);
    connect(&mRescanButton, &QPushButton::clicked, this, &MidiConfigTab::rescan);

}

void MidiConfigTab::apply(Config::Midi &midiConfig) {
    midiConfig.enabled = mMidiGroup.isChecked();
    if (midiConfig.enabled) {
        midiConfig.backendIndex = mApiCombo.currentIndex();
        midiConfig.portIndex = mPortCombo.currentIndex();
    }

    clean();
}

void MidiConfigTab::resetControls(Config::Midi const& midiConfig) {


    auto &prober = MidiProber::instance();

    if (mApiCombo.currentIndex() != midiConfig.backendIndex) {
        prober.setBackend(midiConfig.backendIndex);
        mApiCombo.setCurrentIndex(midiConfig.backendIndex);
    }

    mPortCombo.setCurrentIndex(midiConfig.portIndex);
    mMidiGroup.setChecked(midiConfig.enabled);

    clean();
}

void MidiConfigTab::rescan() {
    mRescanning = true;

    auto &prober = MidiProber::instance();
    auto const current = mPortCombo.currentText();
    prober.probe();
    mPortCombo.clear();
    auto const portNames = prober.portNames();
    mPortCombo.addItems(portNames);
    mPortCombo.setCurrentIndex(portNames.indexOf(current));
    mPortCombo.setEnabled(mPortCombo.count() > 0);

    mRescanning = false;
}

void MidiConfigTab::portChosen() {
    if (!mRescanning) {
        setDirty();
    }
}
