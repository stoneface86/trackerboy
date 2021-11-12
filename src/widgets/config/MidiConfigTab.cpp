
#include "widgets/config/MidiConfigTab.hpp"

#include "midi/MidiEnumerator.hpp"

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSignalBlocker>

MidiConfigTab::MidiConfigTab(MidiConfig const& midiConfig, MidiEnumerator &enumerator, QWidget *parent) :
    ConfigTab(parent),
    mEnumerator(enumerator)
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
    mApiErrorLabel = new QLabel(tr("API Unavailable"));
    rescanLayout->addWidget(mApiErrorLabel);
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

    mApiCombo->addItems(enumerator.backendNames());
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

    auto const current = mPortCombo->currentText();
    auto const backend = mApiCombo->currentIndex();
    mEnumerator.populate(backend);
    mPortCombo->clear();
    auto const portNames = mEnumerator.deviceNames(backend);
    mPortCombo->addItems(portNames);
    auto index = portNames.indexOf(current);
    if (index == -1) {
        // the previous port was not found, so the config is now dirty
        setDirty<Config::CategoryMidi>();
    }
    mPortCombo->setCurrentIndex(index);
    mPortCombo->setEnabled(mPortCombo->count() > 0);
    mApiErrorLabel->setVisible(!mEnumerator.backendIsAvailable(index));
}


void MidiConfigTab::setApi(int index) {
    mEnumerator.populate(index);
    mPortCombo->clear();
    mPortCombo->addItems(mEnumerator.deviceNames(index));
    mPortCombo->setEnabled(mPortCombo->count() > 0);

    mApiErrorLabel->setVisible(!mEnumerator.backendIsAvailable(index));

    setDirty<Config::CategoryMidi>();
}

