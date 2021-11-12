
#pragma once

#include "widgets/config/ConfigTab.hpp"

class MidiEnumerator;

class QComboBox;
class QGroupBox;
class QLabel;


class MidiConfigTab : public ConfigTab {

    Q_OBJECT

public:
    explicit MidiConfigTab(MidiConfig const& midiConfig, MidiEnumerator &enumerator, QWidget *parent = nullptr);

    void apply(MidiConfig &midiConfig);

private:

    void rescan();

    void setApi(int index);

    MidiEnumerator &mEnumerator;

    QGroupBox *mMidiGroup;
    QComboBox *mApiCombo;
    QComboBox *mPortCombo;
    QLabel *mApiErrorLabel;


};
