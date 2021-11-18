#pragma once

#include "config/tabs/ConfigTab.hpp"

class PianoInput;

class QComboBox;
class QGroupBox;

// defined in KeyboardConfigTab
class BindingEdit;

class KeyboardConfigTab : public ConfigTab {

    Q_OBJECT

public:
    explicit KeyboardConfigTab(PianoInput const& input, QWidget *parent = nullptr);

    void apply(PianoInput &input);

private:


    QComboBox *mLayoutCombo;
    QGroupBox *mCustomGroup;

    BindingEdit *mBindingEdit;

};

