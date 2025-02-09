
#pragma once

#include "config/tabs/ConfigTab.hpp"

class GeneralConfig;

class QListWidget;
class QCheckBox;
class QGroupBox;
class QSpinBox;

class GeneralConfigTab : public ConfigTab {

    Q_OBJECT

public:
    explicit GeneralConfigTab(GeneralConfig const& config, QWidget *parent = nullptr);

    void apply(GeneralConfig &config);

private:
    Q_DISABLE_COPY(GeneralConfigTab)

    QListWidget *mOptionList;

    QGroupBox *mAutosaveGroup;
    QSpinBox *mAutosaveIntervalSpin;

    QSpinBox *mPageStepSpin;

};
