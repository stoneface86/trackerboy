
#pragma once

#include "model/SongListModel.hpp"

#include <QAction>
#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QToolBar>
#include <QWidget>


class SongWidget : public QWidget {

    Q_OBJECT

public:
    explicit SongWidget(SongListModel &model, QWidget *parent = nullptr);
    ~SongWidget();

    void setupMenu(QMenu &menu);

private slots:

    void onAddSong();
    void onRemoveSong();
    void onDuplicateSong();
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onTitleEdited(const QString &text);

private:

    void updateActions();

    SongListModel &mModel;

    QMenu mContextMenu;

    QGridLayout mLayout;
        QListView mSongList;
        QToolBar mToolbar;
        QBoxLayout mTitleLayout;
            QLabel mTitleLabel;
            QLineEdit mTitleEdit;

    // these actions don't get parented by mToolbar
    QAction mActionAdd;
    QAction mActionRemove;
    QAction mActionDuplicate;
    QAction mActionMoveUp;
    QAction mActionMoveDown;

};
