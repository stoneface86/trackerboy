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
    ConfigDialog(audio::BackendTable &backendTable, Config &config, QWidget *parent = nullptr);
    ~ConfigDialog();

    void accept() override;

    void reject() override;

    void resetControls();

protected:

    void showEvent(QShowEvent *evt) override;

private slots:
    void bufferSizeSliderChanged(int value);
    void volumeSliderChanged(int value);
    void backendActivated(int index);

    void gainChanged(int channel, int value);
    
private:
    Ui::ConfigDialog *mUi;
    audio::BackendTable &mBackendTable;

    Config &mConfig;
    int mLastBackendIndex;

};
