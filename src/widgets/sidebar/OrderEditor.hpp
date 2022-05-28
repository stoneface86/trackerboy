
#pragma once

#include "model/PatternModel.hpp"
#include "widgets/sidebar/OrderGrid.hpp"
#include "verdigris/wobjectdefs.h"

class QToolBar;
class QScrollBar;
#include <QWidget>

//
// Composite widget for the song order editor, located in the Sidebar
//
class OrderEditor : public QWidget {

    W_OBJECT(OrderEditor)

public:

    struct Actions {
        QAction *add, *remove, *duplicate, *moveUp, *moveDown;
    };

    explicit OrderEditor(PatternModel &model, QWidget *parent = nullptr);

    OrderGrid* grid();

    void addActionsToToolbar(Actions const& actions);

protected:

    virtual void contextMenuEvent(QContextMenuEvent *evt) override;

    virtual void wheelEvent(QWheelEvent *evt) override;

private:

    Q_DISABLE_COPY(OrderEditor)

    QToolBar *mToolbar;
    OrderGrid *mGrid;

    QScrollBar *mScrollbar;
};
