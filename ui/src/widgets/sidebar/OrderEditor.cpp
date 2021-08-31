
#include "widgets/sidebar/OrderEditor.hpp"
#include "core/IconManager.hpp"

#include <QAction>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QScrollBar>
#include <QWheelEvent>

OrderEditor::OrderEditor(PatternModel &model, QWidget *parent) :
    QWidget(parent),
    mChangeAll(nullptr),
    mGrid(nullptr),
    mScrollbar(nullptr)
{
    auto toolbarLayout = new QHBoxLayout;
    auto toolbar = new QToolBar;
    mChangeAll = new QCheckBox(tr("Change all"));
    toolbarLayout->addWidget(toolbar, 1);
    toolbarLayout->addWidget(mChangeAll);
    
    mGrid = new OrderGrid(model);
    auto gridLayout = new QHBoxLayout;
    mScrollbar = new QScrollBar(Qt::Vertical);
    gridLayout->addWidget(mGrid, 1);
    gridLayout->addWidget(mScrollbar);
    gridLayout->setSpacing(0);

    auto layout = new QVBoxLayout;
    layout->addLayout(toolbarLayout);
    layout->addLayout(gridLayout, 1);
    setLayout(layout);
    
    
    auto incrementAction = new QAction(tr("Increment selection"), this);
    incrementAction->setStatusTip(tr("Increments all selected cells by 1"));
    incrementAction->setIcon(IconManager::getIcon(Icons::increment));
    auto decrementAction = new QAction(tr("Decrement selection"), this);
    decrementAction->setStatusTip(tr("Decrements all selected cells by 1"));
    decrementAction->setIcon(IconManager::getIcon(Icons::decrement));
    toolbar->addAction(incrementAction);
    toolbar->addAction(decrementAction);
    toolbar->setIconSize(IconManager::size());

    mScrollbar->setRange(0, model.patterns() - 1);
    mScrollbar->setPageStep(1);
    connect(mScrollbar, &QScrollBar::valueChanged, &model, &PatternModel::setCursorPattern);
    connect(&model, &PatternModel::cursorPatternChanged, mScrollbar, &QScrollBar::setValue);
    connect(&model, &PatternModel::patternCountChanged, this,
        [this](int count) {
            mScrollbar->setMaximum(count - 1);
        });
    
    connect(mChangeAll, &QCheckBox::toggled, mGrid, &OrderGrid::setChangeAll);
    // connect(incrementAction, &QAction::triggered, this,
    //     [this]() {
            
    //     });

    // connect(decrementAction, &QAction::triggered, this,
    //     [this]() {
            
    //     });

    // connect(setButton, &QPushButton::clicked, this,
    //     [this]() {
    //     });


}

OrderGrid* OrderEditor::grid() {
    return mGrid;
}

void OrderEditor::wheelEvent(QWheelEvent *evt) {
    // pass this event to the scrollbar if the mouse is under the grid
    if (mGrid->underMouse()) {
        mScrollbar->event(evt);
    }
}
