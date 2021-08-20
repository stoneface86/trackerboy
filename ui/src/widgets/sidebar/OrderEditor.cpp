
#include "widgets/sidebar/OrderEditor.hpp"

#include "misc/utils.hpp"

#include <QAction>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

OrderEditor::OrderEditor(QWidget *parent) :
    QWidget(parent),
    mSetSpin(new CustomSpinBox),
    mOrderView(new QTableView)
{

    auto layout = new QVBoxLayout;

    auto toolbarLayout = new QHBoxLayout;
    auto toolbar = new QToolBar;
    auto setButton = new QPushButton(tr("Set"));
    toolbarLayout->addWidget(toolbar, 1);
    toolbarLayout->addWidget(mSetSpin);
    toolbarLayout->addWidget(setButton);
    
    layout->addLayout(toolbarLayout);
    layout->addWidget(mOrderView, 1);
    setLayout(layout);
    
    
    auto incrementAction = new QAction(this);
    auto decrementAction = new QAction(this);
    setupAction(*incrementAction, "Increment selection", "Increments all selected cells by 1", Icons::increment);
    setupAction(*decrementAction, "Decrement selection", "Decrements all selected cells by 1", Icons::decrement);
    toolbar->addAction(incrementAction);
    toolbar->addAction(decrementAction);
    toolbar->setIconSize(IconManager::size());

    setButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    mOrderView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(mOrderView, &QTableView::customContextMenuRequested, this,
        [this](QPoint const& pos) {
            QPoint mapped = mOrderView->viewport()->mapToGlobal(pos);
            emit popupMenuAt(mapped);
        });
    mOrderView->setTabKeyNavigation(false);
    auto headerView = mOrderView->horizontalHeader();
    headerView->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    auto verticalHeader = mOrderView->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    verticalHeader->setMinimumSectionSize(-1);
    verticalHeader->setDefaultSectionSize(verticalHeader->minimumSectionSize());

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
