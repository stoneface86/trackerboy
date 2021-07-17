
#pragma once

class OrderModel;

#include "widgets/CustomSpinBox.hpp"

#include <QAction>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableView>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>


class OrderEditor : public QWidget {

    Q_OBJECT

public:

    explicit OrderEditor(QWidget *parent = nullptr);

    void setModel(OrderModel *model);


signals:
    void popupMenuAt(QPoint const& pos);

    void jumpToPattern(int pattern);

private:

    void currentIndexChanged(QModelIndex const& index);
    void currentChanged(QModelIndex const &current, QModelIndex const &prev);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    OrderModel *mOrderModel;

    QVBoxLayout mLayout;
        QHBoxLayout mButtonLayout;
            QToolBar mToolbar;
                QAction mActionIncrement;
                QAction mActionDecrement;
            CustomSpinBox mSetSpin;
            QPushButton mSetButton;
        QTableView mOrderView;
};
