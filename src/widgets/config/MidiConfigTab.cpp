
#include "widgets/config/MidiConfigTab.hpp"

#include "core/midi/MidiProber.hpp"

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSignalBlocker>

MidiConfigTab::MidiConfigTab(MidiConfig const& midiConfig, QWidget *parent) :
    ConfigTab(parent)
{

    auto layout = new QVBoxLayout;

    mMidiGroup = new QGroupBox(tr("MIDI input"));
    auto midiLayout = new QGridLayout;
    midiLayout->addWidget(new QLabel(tr("API")), 0, 0);
    mApiCombo = new QComboBox;
    midiLayout->addWidget(mApiCombo, 0, 1);
    midiLayout->addWidget(new QLabel(tr("Device")), 1, 0);
    mPortCombo = new QComboBox;
    midiLayout->addWidget(mPortCombo, 1, 1);

    auto rescanLayout = new QHBoxLayout;
    rescanLayout->addStretch();
    auto rescanButton = new QPushButton(tr("Rescan"));
    rescanLayout->addWidget(rescanButton);

    midiLayout->addLayout(rescanLayout, 2, 0, 1, 2);
    midiLayout->setColumnStretch(1, 1);
    mMidiGroup->setLayout(midiLayout);


    layout->addWidget(mMidiGroup);
    layout->addStretch(1);
    setLayout(layout);

    mMidiGroup->setCheckable(true);
    mMidiGroup->setChecked(midiConfig.isEnabled());

    auto &prober = MidiProber::instance();
    mApiCombo->addItems(prober.backendNames());
    mApiCombo->setCurrentIndex(midiConfig.backendIndex());
    setApi(midiConfig.backendIndex());
    clean();


    connect(mApiCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &MidiConfigTab::setApi);
    connect(mPortCombo, qOverload<int>(&QComboBox::activated), this, &MidiConfigTab::setDirty<Config::CategoryMidi>);
    connect(mMidiGroup, &QGroupBox::toggled, this, &MidiConfigTab::setDirty<Config::CategoryMidi>);
    connect(rescanButton, &QPushButton::clicked, this, &MidiConfigTab::rescan);

}

void MidiConfigTab::apply(MidiConfig &midiConfig) {
    auto const enabled = mMidiGroup->isChecked();
    midiConfig.setEnabled(enabled);
    if (enabled) {
        midiConfig.setBackendIndex(mApiCombo->currentIndex());
        midiConfig.setPortIndex(mPortCombo->currentIndex());
    }

    clean();
}

void MidiConfigTab::rescan() {
    QSignalBlocker blocker(mPortCombo);

    auto &prober = MidiProber::instance();
    auto const current = mPortCombo->currentText();
    prober.probe();
    mPortCombo->clear();
    auto const portNames = prober.portNames();
    mPortCombo->addItems(portNames);
    auto index = portNames.indexOf(current);
    if (index == -1) {
        // the previous port was not found, so the config is now dirty
        setDirty<Config::CategoryMidi>();
    }
    mPortCombo->setCurrentIndex(index);
    mPortCombo->setEnabled(mPortCombo->count() > 0);
}


void MidiConfigTab::setApi(int index) {
    auto &prober = MidiProber::instance();
    prober.setBackend(index);
    mPortCombo->clear();
    mPortCombo->addItems(prober.portNames());
    mPortCombo->setEnabled(mPortCombo->count() > 0);

    setDirty<Config::CategoryMidi>();
}

