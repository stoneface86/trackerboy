
#include "widgets/docks/ModulePropertiesWidget.hpp"


ModulePropertiesWidget::ModulePropertiesWidget(QWidget *parent) :
    QWidget(parent),
    mLayout(QBoxLayout::TopToBottom),
    mTitleEdit(),
    mArtistEdit(),
    mCopyrightEdit(),
    mCommentsEdit()
{

    mLayout.addWidget(&mTitleEdit);
    mLayout.addWidget(&mArtistEdit);
    mLayout.addWidget(&mCopyrightEdit);
    mLayout.addWidget(&mCommentsEdit, 1);
    setLayout(&mLayout);

    mTitleEdit.setPlaceholderText(tr("Title"));
    mTitleEdit.setMaxLength(32);

    mArtistEdit.setPlaceholderText(tr("Artist"));
    mArtistEdit.setMaxLength(32);

    mCopyrightEdit.setPlaceholderText(tr("Copyright"));
    mCopyrightEdit.setMaxLength(32);

    mCommentsEdit.setPlaceholderText(tr("Comments"));

}

ModulePropertiesWidget::~ModulePropertiesWidget() {
}
