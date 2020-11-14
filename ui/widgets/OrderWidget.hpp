#pragma once

#include "model/OrderModel.hpp"

#include <QWidget>

namespace Ui {
class OrderWidget;
}

class OrderWidget : public QWidget {

    Q_OBJECT

public:
    explicit OrderWidget(QWidget *parent = nullptr);
    virtual ~OrderWidget();

    void setModel(OrderModel *model);

private slots:

    void currentChanged(QModelIndex const &current, QModelIndex const &prev);

    void increment();
    void decrement();
    void set();

private:

    Ui::OrderWidget *mUi;

    OrderModel *mModel;
};
