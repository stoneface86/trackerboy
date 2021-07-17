
#include "widgets/sidebar/OrderEditor.hpp"
#include "core/model/OrderModel.hpp"

#include "misc/utils.hpp"

#include <QHeaderView>
#include <QMenu>

OrderEditor::OrderEditor(QWidget *parent) :
    QWidget(parent),
    mOrderModel(nullptr),
    mLayout(),
    mButtonLayout(),
    mToolbar(),
    mActionIncrement(),
    mActionDecrement(),
    mSetSpin(),
    mSetButton(tr("Set")),
    mOrderView()
{


    mButtonLayout.addWidget(&mToolbar, 1);
    mButtonLayout.addWidget(&mSetSpin);
    mButtonLayout.addWidget(&mSetButton);

    mLayout.addLayout(&mButtonLayout);
    mLayout.addWidget(&mOrderView, 1);
    setLayout(&mLayout);

    setupAction(mActionIncrement, "Increment selection", "Increments all selected cells by 1", Icons::increment);
    setupAction(mActionDecrement, "Decrement selection", "Decrements all selected cells by 1", Icons::decrement);
    mToolbar.addAction(&mActionIncrement);
    mToolbar.addAction(&mActionDecrement);
    mToolbar.setIconSize(IconManager::size());

    mSetButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    mOrderView.setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(&mOrderView, &QTableView::customContextMenuRequested, this,
        [this](QPoint const& pos) {
            QPoint mapped = mOrderView.viewport()->mapToGlobal(pos);
            emit popupMenuAt(mapped);
        });
    mOrderView.setTabKeyNavigation(false);
    auto headerView = mOrderView.horizontalHeader();
    headerView->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    auto verticalHeader = mOrderView.verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    verticalHeader->setMinimumSectionSize(-1);
    verticalHeader->setDefaultSectionSize(verticalHeader->minimumSectionSize());

    connect(&mActionIncrement, &QAction::triggered, this,
        [this]() {
            mOrderModel->incrementSelection(mOrderView.selectionModel()->selection());
        });

    connect(&mActionDecrement, &QAction::triggered, this,
        [this]() {
            mOrderModel->decrementSelection(mOrderView.selectionModel()->selection());
        });

    connect(&mSetButton, &QPushButton::clicked, this,
        [this]() {
            mOrderModel->setSelection(
                mOrderView.selectionModel()->selection(),
                static_cast<uint8_t>(mSetSpin.value())
            );
        });


}

void OrderEditor::setModel(OrderModel *model) {
    Q_ASSERT(model && mOrderModel == nullptr); // this function should only be called once
    
    mOrderModel = model;
    mOrderView.setModel(model);
    connect(model, &OrderModel::currentIndexChanged, this, &OrderEditor::currentIndexChanged);
    auto selectionModel = mOrderView.selectionModel();
    connect(selectionModel, &QItemSelectionModel::currentChanged, this, &OrderEditor::currentChanged);
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &OrderEditor::selectionChanged);
    selectionModel->select(model->currentIndex(), QItemSelectionModel::Select);

}

void OrderEditor::currentIndexChanged(QModelIndex const& index) {
    mOrderView.selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
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
    auto model = mOrderView.selectionModel();
    if (!model->hasSelection()) {
        // user deselected everything, force selection of the current index
        model->select(mOrderView.currentIndex(), QItemSelectionModel::Select);
    }
    
}
