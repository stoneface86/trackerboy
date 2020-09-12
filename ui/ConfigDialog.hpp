#pragma once

#include <QDialog>
#include <QSettings>

#pragma warning(push, 0)
#include "designer/ui_ConfigDialog.h"
#pragma warning(pop)

#include "audio.hpp"

class ConfigDialog : public QDialog, private Ui::ConfigDialog {

    Q_OBJECT

public:
    ConfigDialog(QWidget *parent = nullptr);


    void loadSettings(QSettings &settings);

    void saveSettings(QSettings &settings);

    void accept() override;

private slots:
    void bufferSizeSliderChanged(int value);
    void volumeSliderChanged(int value);
    void hostApiSelected(int index);
    void deviceSelected(int index);
    void samplerateSelected(int index);

    void gainChanged(int channel, int value);

private:

    audio::DeviceManager mDeviceManager;
    std::vector<audio::Samplerate> mSamplerateVec;
    audio::Samplerate mLastSamplerate;
    bool mIgnoreSamplerateSelection;

};
