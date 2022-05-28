#pragma once

#include "config/tabs/ConfigTab.hpp"
#include "verdigris/wobjectdefs.h"

class PianoInput;
class ShortcutTable;

class QComboBox;
class QGroupBox;
class QKeySequenceEdit;
class QPushButton;

// defined in KeyboardConfigTab
class BindingEdit;
class ShortcutTableModel;

class KeyboardConfigTab : public ConfigTab {

    W_OBJECT(KeyboardConfigTab)

public:
    explicit KeyboardConfigTab(PianoInput &input, ShortcutTable &shortcuts, QWidget *parent = nullptr);

    void apply(PianoInput &input);

private:

    void setKeyEditEnable(bool enable);

    PianoInput &mInput;
    ShortcutTable &mShortcuts;

    QComboBox *mLayoutCombo;
    QGroupBox *mCustomGroup;

    BindingEdit *mBindingEdit;
    QKeySequenceEdit *mKeyEdit;
    QPushButton *mClearButton;
    QPushButton *mDefaultButton;

    ShortcutTableModel *mModel;

    int mSelectedShortcut;

};

