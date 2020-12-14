#pragma once

#include "model/OrderModel.hpp"

#include <QAction>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>


//
// Composite widget for the order editor.
//
class OrderWidget : public QWidget {

    Q_OBJECT

public:

    OrderWidget(OrderModel &model, QWidget *parent = nullptr);
    ~OrderWidget();

    void setupMenu(QMenu &menu);

private slots:

    void currentChanged(QModelIndex const &current, QModelIndex const &prev);

    void increment();
    void decrement();
    void set();

    void tableViewContextMenu(QPoint pos);

private:
    OrderModel &mModel;

    QMenu mContextMenu;
    QAction mActionInsert;
    QAction mActionRemove;
    QAction mActionDuplicate;
    QAction mActionMoveUp;
    QAction mActionMoveDown;

    QVBoxLayout mLayout;
        QHBoxLayout mLayoutOperations;
            QPushButton mIncrementButton;
            QPushButton mDecrementButton;
            QLineEdit mSetEdit;
            QPushButton mSetButton;
        QTableView mOrderView;


};
