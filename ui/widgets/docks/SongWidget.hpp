
#pragma once

#include "model/SongListModel.hpp"

#include <QWidget>

namespace Ui {
class SongWidget;
}

class SongWidget : public QWidget {

    Q_OBJECT

public:
    explicit SongWidget(SongListModel &model, QWidget *parent = nullptr);
    ~SongWidget();


private:
    Ui::SongWidget *mUi;

    SongListModel &mModel;
};
