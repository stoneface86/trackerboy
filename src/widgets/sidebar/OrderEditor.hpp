
#pragma once

#include "model/PatternModel.hpp"
#include "widgets/sidebar/OrderGrid.hpp"

#include <QCheckBox>
#include <QPoint>
#include <QScrollBar>
#include <QTableView>

//
// Composite widget for the song order editor, located in the Sidebar
//
class OrderEditor : public QWidget {

    Q_OBJECT

public:

    explicit OrderEditor(PatternModel &model, QWidget *parent = nullptr);

    OrderGrid* grid();

signals:
    //
    // re-emits QTableView's contextMenuRequested signal
    //
    void popupMenuAt(QPoint const& pos);

protected:

    virtual void wheelEvent(QWheelEvent *evt) override;

private:

    Q_DISABLE_COPY(OrderEditor)

    QCheckBox *mChangeAll;
    OrderGrid *mGrid;

    QScrollBar *mScrollbar;
};
