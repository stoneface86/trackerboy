
#pragma once

#include <QWidget>

namespace Ui {
class ModulePropertiesWidget;
}

class ModulePropertiesWidget : public QWidget {

    Q_OBJECT

public:
    explicit ModulePropertiesWidget(QWidget *parent = nullptr);
    virtual ~ModulePropertiesWidget();


private:
    Ui::ModulePropertiesWidget *mUi;

};
