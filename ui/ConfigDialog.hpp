#pragma once

#include <QDialog>
#include <QSettings>

#pragma warning(push, 0)
#include "designer/ui_ConfigDialog.h"
#pragma warning(pop)

#include "audio.hpp"

#include "Config.hpp"

class ConfigDialog : public QDialog, private Ui::ConfigDialog {

    Q_OBJECT

public:
    ConfigDialog(Config &config, QWidget *parent = nullptr);

    void accept() override;

    void reject() override;

private slots:
    void bufferSizeSliderChanged(int value);
    void volumeSliderChanged(int value);
    void hostApiSelected(int index);
    void deviceSelected(int index);
    void samplerateSelected(int index);

    void gainChanged(int channel, int value);
    
private:

    void fillDeviceCombo(int hostIndex);
    void fillSamplerateCombo();

    void resetControls();

    Config &mConfig;

    audio::DeviceManager mDeviceManager;
    bool mIgnoreSelections;

};
