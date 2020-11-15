#pragma once

#include "model/OrderModel.hpp"
#include "misc/OrderActions.hpp"

#include <QMenu>
#include <QWidget>

namespace Ui {
class OrderWidget;
}

class OrderWidget : public QWidget {

    Q_OBJECT

public:
    explicit OrderWidget(QWidget *parent = nullptr);
    virtual ~OrderWidget();

    void init(OrderModel *model, QMenu *menu);

private slots:

    void currentChanged(QModelIndex const &current, QModelIndex const &prev);

    void increment();
    void decrement();
    void set();

    void tableViewContextMenu(QPoint pos);

private:

    Ui::OrderWidget *mUi;

    OrderModel *mModel;
    QMenu *mContextMenu;
};
