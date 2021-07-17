
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
    mOrderModel(nullptr),
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

    connect(incrementAction, &QAction::triggered, this,
        [this]() {
            mOrderModel->incrementSelection(mOrderView->selectionModel()->selection());
        });

    connect(decrementAction, &QAction::triggered, this,
        [this]() {
            mOrderModel->decrementSelection(mOrderView->selectionModel()->selection());
        });

    connect(setButton, &QPushButton::clicked, this,
        [this]() {
            mOrderModel->setSelection(
                mOrderView->selectionModel()->selection(),
                static_cast<uint8_t>(mSetSpin->value())
            );
        });


}

void OrderEditor::setModel(OrderModel *model) {
    Q_ASSERT(model && mOrderModel == nullptr); // this function should only be called once
    
    mOrderModel = model;
    mOrderView->setModel(model);
    connect(model, &OrderModel::currentIndexChanged, this, &OrderEditor::currentIndexChanged);
    auto selectionModel = mOrderView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::currentChanged, this, &OrderEditor::currentChanged);
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &OrderEditor::selectionChanged);
    selectionModel->select(model->currentIndex(), QItemSelectionModel::Select);

}

void OrderEditor::currentIndexChanged(QModelIndex const& index) {
    mOrderView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
}

void OrderEditor::currentChanged(QModelIndex const &current, QModelIndex const &prev) {
    Q_UNUSED(prev);

    // auto &patternModel = mDocument.patternModel();
    // if (patternModel.isPlaying() && patternModel.isFollowing()) {
    //     if (orderModel.currentPattern() != current.row()) {
    //         // jump to this pattern instead of selecting it
    //         emit patternJumpRequested(current.row());
    //     }
    //     orderModel.selectTrack(current.column());
    // } else {
        mOrderModel->select(current.row(), current.column());
    // }
    
}

void OrderEditor::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    // this slot is just for preventing the user from deselecting
    auto model = mOrderView->selectionModel();
    if (!model->hasSelection()) {
        // user deselected everything, force selection of the current index
        model->select(mOrderView->currentIndex(), QItemSelectionModel::Select);
    }
    
}
