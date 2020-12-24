
#include "ConfigDialog.hpp"
#include "core/samplerates.hpp"

#include <QMessageBox>
#include <QPushButton>

#include <algorithm>
#include <cmath>


ConfigDialog::ConfigDialog(Config &config, QWidget *parent) :
    QDialog(parent),
    mConfig(config),
    mDirty(Config::CategoryNone),
    mLayout(),
    mTabs(),
    mTabSound(config),
    mButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply, Qt::Horizontal)
{
    // layout
    mTabs.addTab(&mTabSound, tr("Sound"));

    mLayout.addWidget(&mTabs, 1);
    mLayout.addWidget(&mButtons);

    mLayout.setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    setLayout(&mLayout);

    setWindowTitle(tr("Configuration"));

    connect(&mTabSound, &SoundConfigTab::dirty, this, &ConfigDialog::setDirty);

    auto applyButton = mButtons.button(QDialogButtonBox::Apply);
    applyButton->setEnabled(false);
    connect(applyButton, &QPushButton::clicked, this, &ConfigDialog::apply);
    connect(&mButtons, &QDialogButtonBox::accepted, this, &ConfigDialog::accept);
    connect(&mButtons, &QDialogButtonBox::rejected, this, &ConfigDialog::reject);

}

ConfigDialog::~ConfigDialog() {
}

void ConfigDialog::accept() {
    apply();
    QDialog::accept();
}

void ConfigDialog::reject() {
    // reset all settings
    resetControls();

    QDialog::reject();
}

void ConfigDialog::apply() {
    // update all changes to the Config object

    if (mDirty.testFlag(Config::CategorySound)) {
        mTabSound.apply(mConfig.mSound);        
    }

    emit applied(mDirty);
    clean();
}

void ConfigDialog::showEvent(QShowEvent *evt) {
    mTabs.setCurrentIndex(0);
    QDialog::showEvent(evt);
}


void ConfigDialog::resetControls() {

    mTabSound.resetControls(mConfig.mSound);

    clean();
}

void ConfigDialog::setDirty(Config::Category category) {
    if (!mDirty) {
        mButtons.button(QDialogButtonBox::Apply)->setEnabled(true);
    }
    mDirty |= category;
}

void ConfigDialog::clean() {
    mDirty = Config::CategoryNone;
    mButtons.button(QDialogButtonBox::Apply)->setEnabled(false);
}


