#pragma once

#include "widgets/config/ConfigTab.hpp"

class PianoInput;

class QComboBox;
class QGroupBox;

// defined in KeyboardConfigTab
class BindingEdit;

class KeyboardConfigTab : public ConfigTab {

    Q_OBJECT

public:
    explicit KeyboardConfigTab(QWidget *parent = nullptr);


    void apply(PianoInput &input);

    void resetControls(PianoInput const& input);

private:


    QComboBox *mLayoutCombo;
    QGroupBox *mCustomGroup;

    BindingEdit *mBindingEdit;

};

