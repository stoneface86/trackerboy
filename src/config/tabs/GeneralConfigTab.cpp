
#include "config/tabs/GeneralConfigTab.hpp"

#include "config/data/GeneralConfig.hpp"
#include "utils/connectutils.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QListWidget>
#include <QLabel>

#include <QtDebug>

#include <array>

#define TU GeneralConfigTabTU
namespace TU {

static std::array<const char*, GeneralConfig::OptionCount> const OPTION_TEXT = {
    QT_TR_NOOP("Backup module on save"),
    QT_TR_NOOP("Wrap cursor"),
    QT_TR_NOOP("Wrap cursor across patterns"),
    QT_TR_NOOP("Show row numbers in hexadecimal"),
    QT_TR_NOOP("Show flat accidentals instead of sharps"),
    QT_TR_NOOP("Show pattern previews")
};

}


GeneralConfigTab::GeneralConfigTab(GeneralConfig const& config, QWidget *parent) :
    ConfigTab(parent)
{

    auto layout = new QHBoxLayout;

    auto optionGroup = new QGroupBox(tr("Options"));
    auto optionLayout = new QVBoxLayout;
    mOptionList = new QListWidget;
    optionLayout->addWidget(mOptionList);
    optionGroup->setLayout(optionLayout);

    auto sideLayout = new QVBoxLayout;
    
    // auto-save
    mAutosaveGroup = new QGroupBox(tr("Auto-save"));
    mAutosaveGroup->setCheckable(true);
    mAutosaveGroup->setChecked(config.hasAutosave());
    auto autosaveLayout = new QHBoxLayout;
    autosaveLayout->addWidget(new QLabel(tr("Interval")));
    mAutosaveIntervalSpin = new QSpinBox;
    mAutosaveIntervalSpin->setRange(1, 600);
    mAutosaveIntervalSpin->setValue(config.autosaveInterval());
    mAutosaveIntervalSpin->setSuffix(tr(" s"));
    autosaveLayout->addWidget(mAutosaveIntervalSpin);
    mAutosaveGroup->setLayout(autosaveLayout);

    // page step
    auto pageStepGroup = new QGroupBox(tr("Page step"));
    auto pageStepLayout = new QHBoxLayout;
    pageStepLayout->addWidget(new QLabel(tr("Rows")));
    mPageStepSpin = new QSpinBox;
    mPageStepSpin->setRange(1, 64);
    mPageStepSpin->setValue(config.pageStep());
    pageStepLayout->addWidget(mPageStepSpin);
    pageStepGroup->setLayout(pageStepLayout);

    sideLayout->addWidget(mAutosaveGroup);
    sideLayout->addWidget(pageStepGroup);
    sideLayout->addStretch(1);

    layout->addWidget(optionGroup, 1);
    layout->addLayout(sideLayout);
    setLayout(layout);

    // setup the option list
    for (int i = 0; i < GeneralConfig::OptionCount; ++i) {
        auto item = new QListWidgetItem;
        item->setText(tr(TU::OPTION_TEXT[i]));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setCheckState(config.hasOption((GeneralConfig::Options)i) ? Qt::Checked : Qt::Unchecked);
        mOptionList->addItem(item);
    }

    lazyconnect(mOptionList, itemChanged, this, setDirty<Config::CategoryGeneral>);
    lazyconnect(mAutosaveGroup, toggled, this, setDirty<Config::CategoryGeneral>);
    connect(mAutosaveIntervalSpin, qOverload<int>(&QSpinBox::valueChanged), this, &GeneralConfigTab::setDirty<Config::CategoryGeneral>);
    connect(mPageStepSpin, qOverload<int>(&QSpinBox::valueChanged), this, &GeneralConfigTab::setDirty<Config::CategoryGeneral>);
    
}

void GeneralConfigTab::apply(GeneralConfig &config) {
    config.setAutosave(mAutosaveGroup->isChecked());
    config.setAutosaveInterval(mAutosaveIntervalSpin->value());
    config.setPageStep(mPageStepSpin->value());

    for (int i = 0; i < GeneralConfig::OptionCount; ++i) {
        auto item = mOptionList->item(i);
        config.setOption((GeneralConfig::Options)i, item->checkState() == Qt::Checked);
    }

    clean();
}

#undef TU
