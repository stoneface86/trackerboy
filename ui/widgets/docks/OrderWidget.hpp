#pragma once

#include "misc/SongActions.hpp"
#include "model/OrderModel.hpp"
#include "widgets/CustomSpinBox.hpp"

#include <QAction>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QMenu>
#include <QToolButton>
#include <QTableView>
#include <QToolBar>
#include <QVBoxLayout>
#include <QShowEvent>
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
    QAction mActionIncrement;
    QAction mActionDecrement;

    QVBoxLayout mLayout;
        QHBoxLayout mLayoutSet;
            QToolBar mToolbar;
            CustomSpinBox mSetSpin;
            QToolButton mSetButton;
        QTableView mOrderView;
        


};
