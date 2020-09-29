#pragma once

#include "Config.hpp"

#include "audio.hpp"

#include <QDialog>
#include <QSettings>
#include <QShowEvent>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog {

    Q_OBJECT

public:
    ConfigDialog(Config &config, QWidget *parent = nullptr);
    ~ConfigDialog();

    void accept() override;

    void reject() override;

    void resetControls();

protected:

    void showEvent(QShowEvent *evt) override;

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

    

    Ui::ConfigDialog *mUi;

    Config &mConfig;

    audio::DeviceManager mDeviceManager;
    bool mIgnoreSelections;

};
