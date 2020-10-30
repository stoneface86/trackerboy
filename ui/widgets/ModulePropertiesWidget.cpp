
#include "widgets/ModulePropertiesWidget.hpp"

#pragma warning(push, 0)
#include "ui_ModulePropertiesWidget.h"
#pragma warning(pop)

ModulePropertiesWidget::ModulePropertiesWidget(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::ModulePropertiesWidget())
{
    mUi->setupUi(this);
}

ModulePropertiesWidget::~ModulePropertiesWidget() {
    delete mUi;
}
