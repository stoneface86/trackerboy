
#pragma once

#include "model/OrderModel.hpp"

#include <QScrollBar>
#include <QWidget>

// forward declare these classes
namespace grid {
class PatternGrid;
class PatternGridHeader;
}

class PatternEditor : public QWidget {

    Q_OBJECT

public:
    explicit PatternEditor(OrderModel &model, QWidget *parent = nullptr);
    ~PatternEditor() = default;

private slots:

    void vscrollAction(int action);

private:

    grid::PatternGrid *mGrid;
    grid::PatternGridHeader *mGridHeader;
    QScrollBar *mHScroll;
    QScrollBar *mVScroll;

    

};
