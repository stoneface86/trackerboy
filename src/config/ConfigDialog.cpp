
#include "config/ConfigDialog.hpp"
#include "core/StandardRates.hpp"
#include "utils/connectutils.hpp"

#include "config/tabs/AppearanceConfigTab.hpp"
#include "config/tabs/GeneralConfigTab.hpp"
#include "config/tabs/KeyboardConfigTab.hpp"
#include "config/tabs/SoundConfigTab.hpp"

#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTabWidget>



ConfigDialog::ConfigDialog(
    Config &config,
    AudioEnumerator &audio,
    MidiEnumerator &midi,
    QWidget *parent
) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    mConfig(config),
    mDirty(Config::CategoryNone)
{


    auto tabs = new QTabWidget;

    mGeneral = new GeneralConfigTab(config.general());
    tabs->addTab(mGeneral, tr("General"));

    mAppearance = new AppearanceConfigTab(config.appearance(), config.palette());
    tabs->addTab(mAppearance, tr("Appearance"));

    mKeyboard = new KeyboardConfigTab(config.pianoInput(), config.shortcuts());
    tabs->addTab(mKeyboard, tr("Keyboard"));

    mSound = new SoundConfigTab(config.midi(), config.sound(), audio, midi);
    tabs->addTab(mSound, tr("Sound / MIDI"));

    mButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply, Qt::Horizontal);

    auto layout = new QVBoxLayout;
    layout->addWidget(tabs, 1);
    layout->addWidget(mButtons);
    layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    setLayout(layout);

    setWindowTitle(tr("Configuration"));
    setModal(true);

    auto applyButton = mButtons->button(QDialogButtonBox::Apply);
    applyButton->setEnabled(false);

    lazyconnect(mGeneral, dirty, this, setDirty);
    lazyconnect(mAppearance, dirty, this, setDirty);
    lazyconnect(mKeyboard, dirty, this, setDirty);
    lazyconnect(mSound, dirty, this, setDirty);

    lazyconnect(applyButton, clicked, this, apply);
    lazyconnect(mButtons, accepted, this, accept);
    lazyconnect(mButtons, rejected, this, reject);

}

void ConfigDialog::accept() {
    if (apply()) {
        QDialog::accept();
    }
}

void ConfigDialog::unclean(Config::Categories categories) {
    mDirty = categories;
}

bool ConfigDialog::apply() {
    // update all changes to the Config object

    if (mDirty) {

        if (mDirty.testFlag(Config::CategoryGeneral)) {
            mGeneral->apply(mConfig.general());
        }

        if (mDirty.testFlag(Config::CategoryAppearance)) {
            mAppearance->apply(mConfig.appearance(), mConfig.palette());
        }

        if (mDirty.testFlag(Config::CategoryKeyboard)) {
            mKeyboard->apply(mConfig.pianoInput());
        }

        if (mDirty.testFlag(Config::CategorySound)) {
            mSound->apply(mConfig.sound());
        }

        if (mDirty.testFlag(Config::CategoryMidi)) {
            mSound->apply(mConfig.midi());
        }

        auto dirty = mDirty;
        mDirty = Config::CategoryNone;
        emit applied(dirty);
        if (mDirty) {
            // the receiver had issues with the config, do not accept the dialog
            return false;
        } else {
            mButtons->button(QDialogButtonBox::Apply)->setEnabled(false);
        }

    }

    // ok to accept
    return true;
}

void ConfigDialog::setDirty(Config::Category category) {
    if (!mDirty) {
        mButtons->button(QDialogButtonBox::Apply)->setEnabled(true);
    }
    mDirty |= category;
}

void ConfigDialog::clean() {
    mDirty = Config::CategoryNone;
    mButtons->button(QDialogButtonBox::Apply)->setEnabled(false);
}


