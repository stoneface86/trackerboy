
#include "widgets/TickrateForm.hxx"
#include "utils/connectutils.hxx"

#include <QHBoxLayout>
#include <QRadioButton>
#include <QVBoxLayout>

TickrateForm::TickrateForm(QWidget *parent)
    : QGroupBox(tr("Tick rate"), parent)
    , mButtons{}
    , mCustomRate{} {

    auto layout = new QVBoxLayout;
    mButtons = new QButtonGroup(this);
    auto tickrateDmg = new QRadioButton(tr("DMG (59.7 Hz)"));
    auto tickrateSgb = new QRadioButton(tr("SGB (61.1 Hz)"));
    auto tickrateCustomLayout = new QHBoxLayout;
    auto tickrateCustom = new QRadioButton(tr("Custom: "));
    mCustomRate = new QDoubleSpinBox;
    mCustomRate->setRange(1.0, 131072.0);
    mCustomRate->setSuffix(tr(" Hz"));
    mCustomRate->setDecimals(4);
    mButtons->addButton(tickrateDmg, B::SystemDmg);
    mButtons->addButton(tickrateSgb, B::SystemSgb);
    mButtons->addButton(tickrateCustom, B::SystemCustom);
    layout->addWidget(tickrateDmg);
    layout->addWidget(tickrateSgb);
    tickrateCustomLayout->addWidget(tickrateCustom);
    tickrateCustomLayout->addWidget(mCustomRate, 1);
    layout->addLayout(tickrateCustomLayout);
    setLayout(layout);

    lazyconnect(mCustomRate, valueChanged, this, tickrateChanged);
    connectLambda(mButtons, idToggled, this, [this](int id, bool on) {
        if (on) {
            mCustomRate->setEnabled(id == B::SystemCustom);
            emit tickrateChanged();
        }
    });
}

B::Tickrate TickrateForm::tickrate() const {
    return {(B::System)mButtons->checkedId(), (B::NF32)mCustomRate->value()};
}

void TickrateForm::setTickrate(B::Tickrate const &tickrate) {
    QSignalBlocker blocker(this);
    mCustomRate->setValue(tickrate.customFramerate);
    mCustomRate->setEnabled(tickrate.system == B::SystemCustom);
    mButtons->button((int)tickrate.system)->setChecked(true);
}