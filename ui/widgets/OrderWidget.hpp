#pragma once

#include <QWidget>

namespace Ui {
class OrderWidget;
}

class OrderWidget : public QWidget {

    Q_OBJECT

public:
    explicit OrderWidget(QWidget *parent = nullptr);
    virtual ~OrderWidget();

private:
    Ui::OrderWidget *mUi;
};
