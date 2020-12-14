
#include "widgets/docks/SongWidget.hpp"

#include "misc/IconManager.hpp"

#include <QMessageBox>


SongWidget::SongWidget(SongListModel &model, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mContextMenu(),
    mLayout(),
    mSongList(),
    mToolbar(),
    mTitleLayout(QBoxLayout::LeftToRight),
    mTitleLabel(tr("Title")),
    mTitleEdit(),
    mActionAdd(tr("Add song")),
    mActionRemove(tr("Remove song")),
    mActionDuplicate(tr("Duplicate song")),
    mActionMoveUp(tr("Move song up")),
    mActionMoveDown(tr("Move song down"))
{
    setObjectName(QStringLiteral("SongWidget"));
    mSongList.setModel(&model);

    // layout
    mLayout.addWidget(&mSongList, 0, 0);

    mToolbar.setOrientation(Qt::Vertical);
    mToolbar.setIconSize(QSize(16, 16));
    mToolbar.addAction(&mActionAdd);
    mToolbar.addAction(&mActionRemove);
    mToolbar.addAction(&mActionDuplicate);
    mToolbar.addAction(&mActionMoveUp);
    mToolbar.addAction(&mActionMoveDown);
    mLayout.addWidget(&mToolbar, 0, 1);

    mTitleLayout.addWidget(&mTitleLabel);
    mTitleLayout.addWidget(&mTitleEdit);
    mLayout.addLayout(&mTitleLayout, 1, 0);

    setLayout(&mLayout);

    mTitleEdit.setEnabled(false);
    setupMenu(mContextMenu);

    // actions
    mActionAdd.setStatusTip(tr("Adds a new song to the module"));
    mActionAdd.setIcon(IconManager::getIcon(Icons::itemAdd));
    mActionRemove.setStatusTip(tr("Removes the selected song from the module"));
    mActionRemove.setIcon(IconManager::getIcon(Icons::itemRemove));
    mActionDuplicate.setStatusTip(tr("Makes a copy of the selected song"));
    mActionDuplicate.setIcon(IconManager::getIcon(Icons::itemDuplicate));
    mActionMoveUp.setStatusTip(tr("Move the song up in the list"));
    mActionMoveUp.setIcon(IconManager::getIcon(Icons::moveUp));
    mActionMoveDown.setStatusTip(tr("Move the song down in the list"));
    mActionMoveDown.setIcon(IconManager::getIcon(Icons::moveDown));

    mActionRemove.setEnabled(false);
    mActionDuplicate.setEnabled(false);
    mActionMoveUp.setEnabled(false);
    mActionMoveDown.setEnabled(false);

    // connections
    connect(&mActionAdd, &QAction::triggered, this, &SongWidget::onAddSong);
    connect(&mActionRemove, &QAction::triggered, this, &SongWidget::onRemoveSong);
    connect(&mActionDuplicate, &QAction::triggered, this, &SongWidget::onDuplicateSong);

    connect(&mSongList, &QListView::doubleClicked, &model, qOverload<const QModelIndex&>(&SongListModel::select));
    connect(&mTitleEdit, &QLineEdit::textEdited, this, &SongWidget::onTitleEdited);

    auto selectionModel = mSongList.selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &SongWidget::onSelectionChanged);

    mSongList.setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(&mSongList, &QListView::customContextMenuRequested, this,
        [this](QPoint const &pos) {
            mContextMenu.popup(mSongList.viewport()->mapToGlobal(pos));
        });

}

SongWidget::~SongWidget() {

}

void SongWidget::setupMenu(QMenu &menu) {
    menu.addAction(&mActionAdd);
    menu.addAction(&mActionRemove);
    menu.addAction(&mActionDuplicate);
    menu.addSeparator();
    menu.addAction(&mActionMoveUp);
    menu.addAction(&mActionMoveDown);
}

void SongWidget::onAddSong() {
    mModel.add();
    updateActions();
}

void SongWidget::onRemoveSong() {
    auto result = QMessageBox::question(this,
        "Trackerboy",
        "Do you want to remove this song? There is no undo for this action"
    );

    if (result == QMessageBox::Yes) {
        mModel.remove();
        updateActions();
    }
}

void SongWidget::onDuplicateSong() {
    mModel.duplicate(mSongList.currentIndex().row());
    updateActions();
}

void SongWidget::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    Q_UNUSED(deselected);

    bool hasSelection = !selected.isEmpty();
    mTitleEdit.setEnabled(hasSelection);

    if (hasSelection) {
        mTitleEdit.setText(mModel.nameAt(selected.at(0).top()));
    } else {
        mTitleEdit.clear();
    }

    updateActions();
}

void SongWidget::updateActions() {
    auto selectionModel = mSongList.selectionModel();
    
    auto songs = mModel.rowCount();
    bool canAdd = songs < 256;

    mActionAdd.setEnabled(canAdd);
    

    if (selectionModel->hasSelection()) {
        mActionRemove.setEnabled(songs > 1);
        int index = selectionModel->currentIndex().row();
        mActionDuplicate.setEnabled(canAdd);
        mActionMoveUp.setEnabled(index != 0);
        mActionMoveDown.setEnabled(index != songs - 1);
    } else {
        mActionRemove.setEnabled(false);
        mActionDuplicate.setEnabled(false);
        mActionMoveUp.setEnabled(false);
        mActionMoveDown.setEnabled(false);
    }
}

void SongWidget::onTitleEdited(const QString &text) {
    mModel.rename(mSongList.currentIndex().row(), text);
}
