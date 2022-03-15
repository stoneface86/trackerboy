
#include "widgets/sidebar/OrderEditor.hpp"
#include "utils/connectutils.hpp"
#include "utils/IconLocator.hpp"

#include <QAction>
#include <QHBoxLayout>
#include <QMenu>
#include <QScrollBar>
#include <QToolBar>
#include <QWheelEvent>

OrderEditor::OrderEditor(PatternModel &model, QWidget *parent) :
    QWidget(parent)
{
    mToolbar = new QToolBar;
    
    mGrid = new OrderGrid(model);
    auto gridLayout = new QHBoxLayout;
    mScrollbar = new QScrollBar(Qt::Vertical);
    gridLayout->addWidget(mGrid, 1);
    gridLayout->addWidget(mScrollbar);
    gridLayout->setSpacing(0);

    auto layout = new QHBoxLayout;
    layout->addWidget(mToolbar);
    layout->addLayout(gridLayout, 1);
    setLayout(layout);
    
    auto incrementAction = new QAction(tr("Increment selection"), this);
    incrementAction->setStatusTip(tr("Increments all selected cells by 1"));
    incrementAction->setIcon(IconLocator::get(Icons::increment));
    connectActionTo(incrementAction, mGrid, increment);
    auto decrementAction = new QAction(tr("Decrement selection"), this);
    decrementAction->setStatusTip(tr("Decrements all selected cells by 1"));
    decrementAction->setIcon(IconLocator::get(Icons::decrement));
    connectActionTo(decrementAction, mGrid, decrement);
    auto changeAllAction = new QAction(tr("Change all"), this);
    changeAllAction->setStatusTip(tr("Toggles change all tracks mode"));
    changeAllAction->setIcon(IconLocator::get(Icons::changeAll));
    changeAllAction->setCheckable(true);
    
    mToolbar->addAction(incrementAction);
    mToolbar->addAction(decrementAction);
    mToolbar->addAction(changeAllAction);
    mToolbar->setIconSize(IconLocator::size());
    mToolbar->setOrientation(Qt::Vertical);


    mScrollbar->setRange(0, model.patterns() - 1);
    mScrollbar->setPageStep(1);
    connect(mScrollbar, &QScrollBar::valueChanged, &model, &PatternModel::setCursorPattern);
    connect(&model, &PatternModel::cursorPatternChanged, mScrollbar, &QScrollBar::setValue);
    connect(&model, &PatternModel::patternCountChanged, this,
        [this](int count) {
            mScrollbar->setMaximum(count - 1);
        });
    
    connect(changeAllAction, &QAction::toggled, mGrid, &OrderGrid::setChangeAll);


}

OrderGrid* OrderEditor::grid() {
    return mGrid;
}

void OrderEditor::addActionsToToolbar(Actions const& actions) {
    auto head = mToolbar->actions()[0];
    mToolbar->insertAction(head, actions.add);
    mToolbar->insertAction(head, actions.remove);
    mToolbar->insertAction(head, actions.duplicate);
    mToolbar->insertSeparator(head);
    mToolbar->insertAction(head, actions.moveUp);
    mToolbar->insertAction(head, actions.moveDown);
    mToolbar->insertSeparator(head);
}

void OrderEditor::contextMenuEvent(QContextMenuEvent *evt) {
    if (childAt(evt->pos()) == mGrid) {
        auto menu = new QMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->addActions(mToolbar->actions());
        menu->popup(evt->globalPos());
    }
}

void OrderEditor::wheelEvent(QWheelEvent *evt) {
    // pass this event to the scrollbar if the mouse is under the grid
    if (mGrid->underMouse()) {
        mScrollbar->event(evt);
    }
}
