
#pragma once

#include "model/OrderModel.hpp"
#include "widgets/grid/PatternGrid.hpp"

#include <QScrollBar>
#include <QWidget>

//
// Contains a GridWidget and two scrollbars.
//
class PatternGridPane : public QWidget {

    Q_OBJECT

public:
    explicit PatternGridPane(OrderModel &model, QWidget *parent = nullptr);
    ~PatternGridPane() = default;

    PatternGrid* grid() const;

private slots:

    void vscrollAction(int action);

private:

    PatternGrid *mGrid;
    QScrollBar *mHScroll;
    QScrollBar *mVScroll;

    

};
