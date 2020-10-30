
#include "widgets/SongWidget.hpp"

#pragma warning(push, 0)
#include "ui_SongWidget.h"
#pragma warning(pop)

SongWidget::SongWidget(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::SongWidget()),
    mModel(nullptr)
{
    mUi->setupUi(this);
}

SongWidget::~SongWidget() {
    delete mUi;
}

void SongWidget::init(SongListModel *model) {
    mModel = model;
    mUi->listView->setModel(model);
}
