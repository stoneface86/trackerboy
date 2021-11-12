#pragma once

#include "core/Config.hpp"

#include <QDialog>

class AudioEnumerator;
class MidiEnumerator;
class AppearanceConfigTab;
class KeyboardConfigTab;
class SoundConfigTab;

class QDialogButtonBox;

//
// Configuration dialog. Allows the user to change application settings
//
class ConfigDialog : public QDialog {

    Q_OBJECT

public:

    ConfigDialog(Config &config, AudioEnumerator &audio, MidiEnumerator &midi, QWidget *parent = nullptr);

    virtual void accept() override;

signals:
    // emitted when changes in the Config must be applied
    void applied(Config::Categories categories);

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

    AppearanceConfigTab *mAppearance;
    KeyboardConfigTab *mKeyboard;
    SoundConfigTab *mSound;

    QDialogButtonBox *mButtons;




};


