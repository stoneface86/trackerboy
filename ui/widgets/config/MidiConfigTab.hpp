
#pragma once

#include "widgets/config/ConfigTab.hpp"

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

class MidiConfigTab : public ConfigTab {

    Q_OBJECT

public:
    explicit MidiConfigTab(QWidget *parent = nullptr);

    void apply(Config::Midi &midiConfig);

    void resetControls(Config::Midi const& midiConfig);

private:

    void rescan();

    void portChosen();

    bool mRescanning;

    QVBoxLayout mLayout;
        QGroupBox mMidiGroup;
            QGridLayout mMidiLayout;
                QLabel mApiLabel;
                QComboBox mApiCombo;
                QLabel mPortLabel;
                QComboBox mPortCombo;
                QHBoxLayout mRescanLayout;
                    QPushButton mRescanButton;


};
