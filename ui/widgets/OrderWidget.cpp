#include "widgets/OrderWidget.hpp"

#pragma warning(push, 0)
#include "ui_OrderWidget.h"
#pragma warning(pop)

OrderWidget::OrderWidget(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::OrderWidget())
{
    mUi->setupUi(this);
}

OrderWidget::~OrderWidget() {
    delete mUi;
}
