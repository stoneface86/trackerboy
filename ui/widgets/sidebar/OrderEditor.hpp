
#pragma once

#include "core/model/OrderModel.hpp"
#include "widgets/CustomSpinBox.hpp"

#include <QTableView>

//
// Composite widget for the song order editor, located in the Sidebar
//
class OrderEditor : public QWidget {

    Q_OBJECT

public:

    explicit OrderEditor(QWidget *parent = nullptr);

    void setModel(OrderModel *model);


signals:
    //
    // re-emits QTableView's contextMenuRequested signal
    //
    void popupMenuAt(QPoint const& pos);

    void jumpToPattern(int pattern);

private:

    void currentIndexChanged(QModelIndex const& index);
    void currentChanged(QModelIndex const &current, QModelIndex const &prev);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    OrderModel *mOrderModel;

    CustomSpinBox *mSetSpin;
    QTableView *mOrderView;
};
