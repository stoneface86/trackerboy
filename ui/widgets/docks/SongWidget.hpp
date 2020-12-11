
#pragma once

#include "misc/SongActions.hpp"
#include "model/SongListModel.hpp"

#include <QLineEdit>
#include <QListView>
#include <QWidget>


class SongWidget : public QWidget {

    Q_OBJECT

public:
    explicit SongWidget(SongListModel &model, SongActions actions, QWidget *parent = nullptr);
    ~SongWidget();


private:

    SongListModel &mModel;

    QListView *mSongList;
    QLineEdit *mTitleEdit;

};
