#pragma once

#include "config/tabs/ConfigTab.hpp"

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

    Q_OBJECT

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

