#pragma once

#include "audio/AudioEnumerator.hpp"
#include "config/Config.hpp"
#include "midi/MidiEnumerator.hpp"
#include "verdigris/wobjectdefs.h"

class GeneralConfigTab;
class AppearanceConfigTab;
class KeyboardConfigTab;
class SoundConfigTab;

#include <QDialog>
#include <QDialogButtonBox>

//
// Configuration dialog. Allows the user to change application settings
//
class ConfigDialog : public QDialog {

    W_OBJECT(ConfigDialog)

public:

    explicit ConfigDialog(
        Config &config,
        AudioEnumerator &audio,
        MidiEnumerator &midi,
        QWidget *parent = nullptr
    );

    virtual void accept() override;

    void unclean(Config::Categories categories);

//signals:
    // emitted when changes in the Config must be applied
    void applied(Config::Categories categories) W_SIGNAL(applied, categories)

private:
    Q_DISABLE_COPY(ConfigDialog)

    bool apply();

    void setDirty(Config::Category flag);

    void clean();

    Config &mConfig;


    // configuration categories that are "dirty". A dirty category
    // contains settings changed by the user and must be applied when the
    // user clicks the OK or Apply button.
    Config::Categories mDirty;

    GeneralConfigTab *mGeneral;
    AppearanceConfigTab *mAppearance;
    KeyboardConfigTab *mKeyboard;
    SoundConfigTab *mSound;

    QDialogButtonBox *mButtons;




};


