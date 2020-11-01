#pragma once

#include "Config.hpp"

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
    void apply();

    void bufferSizeSliderChanged(int value);
    void volumeSliderChanged(int value);
    void samplerateActivated(int index);


    
private:
    static constexpr int DIRTY_FLAG_SOUND = 0x1;
    void setDirty(int flag);

    void clean();

    Ui::ConfigDialog *mUi;

    Config &mConfig;
    int mDirtyFlags;

};
