
#pragma once

#include "model/SongListModel.hpp"

#include <QWidget>

namespace Ui {
class SongWidget;
}

class SongWidget : public QWidget {

    Q_OBJECT

public:
    explicit SongWidget(QWidget *parent = nullptr);
    virtual ~SongWidget();

    void init(SongListModel *model);


private:
    Ui::SongWidget *mUi;

    SongListModel *mModel;
};
