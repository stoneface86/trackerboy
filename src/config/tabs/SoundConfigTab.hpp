
#pragma once

#include "config/tabs/ConfigTab.hpp"
class AudioEnumerator;
class MidiEnumerator;
#include "verdigris/wobjectdefs.h"

class QComboBox;
class QGroupBox;
class QSpinBox;
class QLabel;

class DeviceGroup;

//
// Tab widget for the "Sound" tab in ConfigDialog
//
class SoundConfigTab : public ConfigTab {

    W_OBJECT(SoundConfigTab)

public:
    explicit SoundConfigTab(
        MidiConfig const& midiConfig,
        SoundConfig const& soundConfig,
        AudioEnumerator &audio,
        MidiEnumerator &midi,
        QWidget *parent = nullptr
    );

    void apply(SoundConfig &soundConfig);

    void apply(MidiConfig &midiConfig);

private:

    Q_DISABLE_COPY(SoundConfigTab)

    void audioRescan();
    void midiRescan();

    void audioApiChanged(int index);
    void midiApiChanged(int index);

    template <class Enumerator>
    void apiChanged(Enumerator &enumerator, DeviceGroup *group, int index);

    template <class Enumerator>
    void rescan(Enumerator &enumerator, DeviceGroup *group);

    template <class Enumerator>
    void setDirtyFromEnumerator();

    AudioEnumerator &mAudioEnumerator;
    MidiEnumerator &mMidiEnumerator;

    DeviceGroup *mAudioGroup;
    DeviceGroup *mMidiGroup;

    QSpinBox *mLatencySpin;
    QSpinBox *mPeriodSpin;
    QComboBox *mSamplerateCombo;


};
