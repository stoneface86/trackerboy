#pragma once

#include <memory>

#include <QDialog>
#include <QSettings>
#include <QShowEvent>



#include "audio.hpp"

#include "Config.hpp"

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog {

    Q_OBJECT

public:
    ConfigDialog(Config &config, QWidget *parent = nullptr);

    void accept() override;

    void reject() override;

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

    void resetControls();

    std::unique_ptr<Ui::ConfigDialog> mUi;

    Config &mConfig;

    audio::DeviceManager mDeviceManager;
    bool mIgnoreSelections;

};
