
#pragma once

#include <QWidget>

namespace Ui {
class SongPropertiesWidget;
}

class SongPropertiesWidget : public QWidget {

    Q_OBJECT

public:
    explicit SongPropertiesWidget(QWidget *parent = nullptr);
    ~SongPropertiesWidget();


private:
    Ui::SongPropertiesWidget *mUi;

};
