
#include "widgets/config/MidiConfigTab.hpp"

#include "core/midi/MidiProber.hpp"

MidiConfigTab::MidiConfigTab(QWidget *parent) :
    ConfigTab(Config::CategoryMidi, parent),
    mLayout(),
    mMidiGroup(tr("MIDI input")),
    mMidiLayout(),
    mApiLabel(tr("API")),
    mApiCombo(),
    mPortLabel(tr("Device")),
    mPortCombo()
{

    mMidiLayout.addWidget(&mApiLabel, 0, 0);
    mMidiLayout.addWidget(&mApiCombo, 0, 1);
    mMidiLayout.addWidget(&mPortLabel, 1, 0);
    mMidiLayout.addWidget(&mPortCombo, 1, 1);
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

    mMidiGroup.setChecked(midiConfig.enabled);

    auto &prober = MidiProber::instance();

    if (mApiCombo.currentIndex() != midiConfig.backendIndex) {
        prober.setBackend(midiConfig.backendIndex);
        mApiCombo.setCurrentIndex(midiConfig.backendIndex);
    }

    mPortCombo.setCurrentIndex(midiConfig.portIndex);

    clean();
}
