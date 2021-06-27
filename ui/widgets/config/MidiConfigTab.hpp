
#pragma once

#include "widgets/config/ConfigTab.hpp"

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>

class MidiConfigTab : public ConfigTab {

    Q_OBJECT

public:
    explicit MidiConfigTab(QWidget *parent = nullptr);

    void apply(Config::Midi &midiConfig);

    void resetControls(Config::Midi const& midiConfig);

private:

    QVBoxLayout mLayout;
        QGroupBox mMidiGroup;
            QGridLayout mMidiLayout;
                QLabel mApiLabel;
                QComboBox mApiCombo;
                QLabel mPortLabel;
                QComboBox mPortCombo;


};
