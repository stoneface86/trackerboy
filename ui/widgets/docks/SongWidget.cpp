
#include "widgets/docks/SongWidget.hpp"

#pragma warning(push, 0)
#include "ui_SongWidget.h"
#pragma warning(pop)

SongWidget::SongWidget(SongListModel &model, QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::SongWidget()),
    mModel(model)
{
    mUi->setupUi(this);
    mUi->listView->setModel(&model);
}

SongWidget::~SongWidget() {
    delete mUi;
}
