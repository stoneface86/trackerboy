
#include "widgets/TickrateForm.hxx"
#include "utils/connectutils.hxx"

#include <QHBoxLayout>
#include <QRadioButton>
#include <QVBoxLayout>

TickrateForm::TickrateForm(QWidget *parent)
    : QGroupBox(tr("Tick rate"), parent)
    , _buttons(nullptr)
    , _customRate(nullptr) {

    auto layout = new QVBoxLayout;
    _buttons = new QButtonGroup(this);
    auto tickrateDmg = new QRadioButton(tr("DMG (59.7 Hz)"));
    auto tickrateSgb = new QRadioButton(tr("SGB (61.1 Hz)"));
    auto tickrateCustomLayout = new QHBoxLayout;
    auto tickrateCustom = new QRadioButton(tr("Custom: "));
    _customRate = new QDoubleSpinBox;
    _customRate->setRange(1.0, 131072.0);
    _customRate->setSuffix(tr(" Hz"));
    _customRate->setDecimals(4);
    _buttons->addButton(tickrateDmg, B::SystemDmg);
    _buttons->addButton(tickrateSgb, B::SystemSgb);
    _buttons->addButton(tickrateCustom, B::SystemCustom);
    layout->addWidget(tickrateDmg);
    layout->addWidget(tickrateSgb);
    tickrateCustomLayout->addWidget(tickrateCustom);
    tickrateCustomLayout->addWidget(_customRate, 1);
    layout->addLayout(tickrateCustomLayout);
    setLayout(layout);

    lazyconnect(_customRate, valueChanged, this, tickrateChanged);
    connectLambda(_buttons, idToggled, this, [this](int id, bool on) {
        if (on) {
            _customRate->setEnabled(id == B::SystemCustom);
            emit tickrateChanged();
        }
    });
}

B::Tickrate TickrateForm::tickrate() const {
    return {(B::System)_buttons->checkedId(), (B::NF32)_customRate->value()};
}

void TickrateForm::setTickrate(B::Tickrate const &tickrate) {
    QSignalBlocker blocker(this);
    _customRate->setValue(tickrate.customFramerate);
    _customRate->setEnabled(tickrate.system == B::SystemCustom);
    _buttons->button((int)tickrate.system)->setChecked(true);
}