
#pragma once

#include "widgets/config/ConfigTab.hpp"

class QGroupBox;
class QComboBox;


class MidiConfigTab : public ConfigTab {

    Q_OBJECT

public:
    explicit MidiConfigTab(MidiConfig const& midiConfig, QWidget *parent = nullptr);

    void apply(MidiConfig &midiConfig);

private:

    void rescan();

    void setApi(int index);

    QGroupBox *mMidiGroup;
    QComboBox *mApiCombo;
    QComboBox *mPortCombo;


};
