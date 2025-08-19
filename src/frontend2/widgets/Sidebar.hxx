
#pragma once

#include "model/SongListModel.hxx"
#include "model/SongModel.hxx"

#include <QComboBox>
#include <QWidget>

class Sidebar : public QWidget {
    Q_OBJECT

public:
    explicit Sidebar(SongListModel *listModel, SongModel *songModel,
                     QWidget *parent = nullptr);

private:
    Q_DISABLE_COPY(Sidebar)

    void songListReset();

    Document *_document;
    QComboBox *_songCombo;
};
