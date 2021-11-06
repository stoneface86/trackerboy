
#include "ConfigDialog.hpp"
#include "core/StandardRates.hpp"
#include "core/misc/connectutils.hpp"

#include "widgets/config/AppearanceConfigTab.hpp"
#include "widgets/config/KeyboardConfigTab.hpp"
#include "widgets/config/MidiConfigTab.hpp"
#include "widgets/config/SoundConfigTab.hpp"

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTabWidget>



ConfigDialog::ConfigDialog(Config &config, QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    mConfig(config),
    mDirty(Config::CategoryNone)
{


    auto tabs = new QTabWidget;

    mAppearance = new AppearanceConfigTab(config.appearance(), config.palette());
    tabs->addTab(mAppearance, tr("Appearance"));

    mKeyboard = new KeyboardConfigTab(config.pianoInput());
    tabs->addTab(mKeyboard, tr("Keyboard"));

    mMidi = new MidiConfigTab(config.midi());
    tabs->addTab(mMidi, tr("MIDI"));

    mSound = new SoundConfigTab(config.sound());
    tabs->addTab(mSound, tr("Sound"));

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

    lazyconnect(mAppearance, dirty, this, setDirty);
    lazyconnect(mKeyboard, dirty, this, setDirty);
    lazyconnect(mMidi, dirty, this, setDirty);
    lazyconnect(mSound, dirty, this, setDirty);

    lazyconnect(applyButton, clicked, this, apply);
    lazyconnect(mButtons, accepted, this, accept);
    lazyconnect(mButtons, rejected, this, reject);

}

void ConfigDialog::accept() {
    apply();
    QDialog::accept();
}

void ConfigDialog::apply() {
    // update all changes to the Config object

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
        mMidi->apply(mConfig.midi());
    }

    emit applied(mDirty);
    clean();
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


