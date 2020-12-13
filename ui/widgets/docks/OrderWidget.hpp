#pragma once

#include "model/OrderModel.hpp"

#include <QBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QTableView>
#include <QWidget>


//
// Composite widget for the order editor.
//
class OrderWidget : public QWidget {

    Q_OBJECT

public:

    //
    // Container for QActions used within this widget
    //
    struct Actions {

        QAction *insert = nullptr;
        QAction *remove = nullptr;
        QAction *duplicate = nullptr;
        QAction *moveUp = nullptr;
        QAction *moveDown = nullptr;

    };

    OrderWidget(OrderModel &model, QWidget *parent = nullptr);
    ~OrderWidget();

    QMenu* createMenu(QWidget *parent = nullptr);

private slots:

    void currentChanged(QModelIndex const &current, QModelIndex const &prev);

    void increment();
    void decrement();
    void set();

    void tableViewContextMenu(QPoint pos);

private:
    OrderModel &mModel;

    QMenu *mContextMenu;
    Actions mActions;

    QBoxLayout mLayout;
        QBoxLayout mLayoutOperations;
            QPushButton mIncrementButton;
            QPushButton mDecrementButton;
            QLineEdit mSetEdit;
            QPushButton mSetButton;
        QTableView mOrderView;


};
