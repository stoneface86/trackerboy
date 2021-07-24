
#pragma once

#include "widgets/CustomSpinBox.hpp"

#include <QTableView>

//
// Composite widget for the song order editor, located in the Sidebar
//
class OrderEditor : public QWidget {

    Q_OBJECT

public:

    explicit OrderEditor(QWidget *parent = nullptr);


signals:
    //
    // re-emits QTableView's contextMenuRequested signal
    //
    void popupMenuAt(QPoint const& pos);

    void jumpToPattern(int pattern);

private:

    Q_DISABLE_COPY(OrderEditor)

    CustomSpinBox *mSetSpin;
    QTableView *mOrderView;
};
