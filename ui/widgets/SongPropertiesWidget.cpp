
#include "widgets/SongPropertiesWidget.hpp"

#pragma warning(push, 0)
#include "ui_SongPropertiesWidget.h"
#pragma warning(pop)

SongPropertiesWidget::SongPropertiesWidget(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::SongPropertiesWidget())
{
    mUi->setupUi(this);
}

SongPropertiesWidget::~SongPropertiesWidget() {
    delete mUi;
}
