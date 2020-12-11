
#include "widgets/docks/SongWidget.hpp"

#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QToolBar>

SongWidget::SongWidget(SongListModel &model, SongActions actions, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mSongList(new QListView()),
    mTitleEdit(new QLineEdit())
{
    setObjectName(QStringLiteral("SongWidget"));
    mSongList->setModel(&model);

    // layout
    auto grid = new QGridLayout();
    grid->addWidget(mSongList, 0, 0);

    auto toolbar = new QToolBar();
    toolbar->setOrientation(Qt::Vertical);
    toolbar->addAction(actions.add);
    toolbar->addAction(actions.remove);
    //auto moveUpAction = toolbar->addAction(tr("Move up"));
    //auto moveDownAction = toolbar->addAction(tr("Move down"));
    grid->addWidget(toolbar, 0, 1);

    auto editLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    editLayout->addWidget(new QLabel(tr("Title")));
    editLayout->addWidget(mTitleEdit);
    grid->addLayout(editLayout, 1, 0);

    setLayout(grid);

    mTitleEdit->setText(model.name());

    // connections
    connect(mSongList, &QListView::doubleClicked, &model, qOverload<const QModelIndex&>(&SongListModel::select));
    connect(mTitleEdit, &QLineEdit::textEdited, &model, &SongListModel::rename);
    connect(&model, &SongListModel::currentIndexChanged, this,
        [this](int index) {
            if (index != -1) {
                mTitleEdit->setText(mModel.name());
            }
        });

}

SongWidget::~SongWidget() {

}
