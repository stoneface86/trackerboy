#pragma once

#include "core/Config.hpp"
#include "widgets/config/AppearanceConfigTab.hpp"
#include "widgets/config/MidiConfigTab.hpp"
#include "widgets/config/SoundConfigTab.hpp"

#include <QDialog>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>


//
// Configuration dialog. Allows the user to change application settings
//
class ConfigDialog : public QDialog {

    Q_OBJECT

public:

    ConfigDialog(Config &config, QWidget *parent = nullptr);
    ~ConfigDialog();

    void accept() override;

    void reject() override;

    void resetControls();

signals:
    // emitted when changes in the Config must be applied
    void applied(Config::Categories categories);

protected:

    void showEvent(QShowEvent *evt) override;

private slots:
    void apply();

    void setDirty(Config::Category flag);
    
private:
    Q_DISABLE_COPY(ConfigDialog)

    void clean();

    Config &mConfig;
    // configuration categories that are "dirty". A dirty category
    // contains settings changed by the user and must be applied when the
    // user clicks the OK or Apply button.
    Config::Categories mDirty;

    QVBoxLayout mLayout;
        QTabWidget mTabs;
            AppearanceConfigTab mTabAppearance;
            MidiConfigTab mTabMidi;
            SoundConfigTab mTabSound;
        QDialogButtonBox mButtons;



};


