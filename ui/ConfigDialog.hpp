#pragma once

#include <QDialog>

#pragma warning(push, 0)
#include "designer/ui_ConfigDialog.h"
#pragma warning(pop)

class ConfigDialog : public QDialog, private Ui::ConfigDialog {

    Q_OBJECT

public:
    ConfigDialog(QWidget *parent = nullptr);

};
