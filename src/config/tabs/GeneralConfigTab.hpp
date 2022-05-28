
#pragma once

#include "config/tabs/ConfigTab.hpp"
#include "verdigris/wobjectdefs.h"

class GeneralConfig;

class QListWidget;
class QCheckBox;
class QGroupBox;
class QSpinBox;

class GeneralConfigTab : public ConfigTab {

    W_OBJECT(GeneralConfigTab)

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
