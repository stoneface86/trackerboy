
#include "widgets/docks/OrderWidget.hpp"

#include "misc/IconManager.hpp"
#include "misc/utils.hpp"

#include <QFontDatabase>
#include <QHeaderView>
#include <QtDebug>


OrderWidget::OrderWidget(OrderModel &model, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mIgnoreSelect(false),
    mContextMenu(),
    mLayout(),
    mToolbar(),
    mSetButton(),
    mOrderView()
{

    // layout
    mLayoutSet.addWidget(&mToolbar);
    mLayoutSet.addStretch(1);
    mLayoutSet.addWidget(&mSetSpin);
    mLayoutSet.addWidget(&mSetButton);

    mLayout.addLayout(&mLayoutSet);
    mLayout.addWidget(&mOrderView, 1);
    setLayout(&mLayout);
    
    // settings

    mToolbar.setIconSize(QSize(16, 16));
    mToolbar.addAction(&mActionInsert);
    mToolbar.addAction(&mActionRemove);
    mToolbar.addAction(&mActionDuplicate);
    mToolbar.addAction(&mActionMoveUp);
    mToolbar.addAction(&mActionMoveDown);
    mToolbar.addAction(&mActionIncrement);
    mToolbar.addAction(&mActionDecrement);

    mSetSpin.setDigits(2);
    mSetSpin.setRange(0, 255);
    mSetSpin.setDisplayIntegerBase(16);
    
    mSetButton.setText(tr("Set"));
    mSetButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    
    
    mActionRemove.setEnabled(false);
    mActionMoveUp.setEnabled(false);
    mActionMoveDown.setEnabled(false);

    // actions
    setupAction(mActionInsert, "&Insert order row", "Inserts a new order at the current pattern", Icons::itemAdd);
    setupAction(mActionRemove, "&Remove order row", "Removes the order at the current pattern", Icons::itemRemove);
    setupAction(mActionDuplicate, "&Duplicate order row", "Duplicates the order at the current pattern", Icons::itemDuplicate);
    setupAction(mActionMoveUp, "Move order &up", "Moves the order up 1", Icons::moveUp);
    setupAction(mActionMoveDown, "Move order dow&n", "Moves the order down 1", Icons::moveDown);
    setupAction(mActionIncrement, "Increment selection", "Increments all selected cells by 1", Icons::increment);
    setupAction(mActionDecrement, "Decrement selection", "Decrements all selected cells by 1", Icons::decrement);

    connect(&mActionInsert, &QAction::triggered, &model, &OrderModel::insert);
    connect(&model, &OrderModel::canInsert, &mActionInsert, &QAction::setEnabled);

    connect(&mActionRemove, &QAction::triggered, &model, &OrderModel::remove);
    connect(&model, &OrderModel::canRemove, &mActionRemove, &QAction::setEnabled);

    connect(&mActionDuplicate, &QAction::triggered, &model, &OrderModel::duplicate);
    connect(&model, &OrderModel::canInsert, &mActionDuplicate, &QAction::setEnabled);

    
    connect(&mActionMoveUp, &QAction::triggered, &model, &OrderModel::moveUp);
    connect(&model, &OrderModel::canMoveUp, &mActionMoveUp, &QAction::setEnabled);

   
    connect(&mActionMoveDown, &QAction::triggered, &model, &OrderModel::moveDown);
    connect(&model, &OrderModel::canMoveDown, &mActionMoveDown, &QAction::setEnabled);

    // menu
    setupMenu(mContextMenu);

    mOrderView.setModel(&model);

    auto selectionModel = mOrderView.selectionModel();
    connect(selectionModel, &QItemSelectionModel::currentChanged, this, &OrderWidget::currentChanged);
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &OrderWidget::selectionChanged);
    connect(&model, &OrderModel::currentIndexChanged, this,
        [this](const QModelIndex &index) {
            if (!mIgnoreSelect) {
                mOrderView.selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
            }
        });

    // initialize selection
    selectionModel->select(model.index(0, 0), QItemSelectionModel::Select);
    
    mOrderView.setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(&mOrderView, &QTableView::customContextMenuRequested, this, &OrderWidget::tableViewContextMenu);
    
    auto headerView = mOrderView.horizontalHeader();
    headerView->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);


    auto verticalHeader = mOrderView.verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    verticalHeader->setMinimumSectionSize(-1);
    verticalHeader->setDefaultSectionSize(verticalHeader->minimumSectionSize());

    connect(&mActionIncrement, &QAction::triggered, this, &OrderWidget::increment);
    connect(&mActionDecrement, &QAction::triggered, this, &OrderWidget::decrement);
    connect(&mSetButton, &QToolButton::clicked, this, &OrderWidget::set);

}

OrderWidget::~OrderWidget() {
}

void OrderWidget::setupMenu(QMenu &menu) {
    menu.addAction(&mActionInsert);
    menu.addAction(&mActionRemove);
    menu.addAction(&mActionDuplicate);
    menu.addSeparator();
    menu.addAction(&mActionMoveUp);
    menu.addAction(&mActionMoveDown);
}

void OrderWidget::currentChanged(QModelIndex const &current, QModelIndex const &prev) {
    Q_UNUSED(prev);

    mIgnoreSelect = true;
    mModel.select(current.row(), current.column());
    mIgnoreSelect = false;
}

void OrderWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    // this slot is just for preventing the user from deselecting
    auto model = mOrderView.selectionModel();
    if (!model->hasSelection()) {
        // user deselected everything, force selection of the current index
        model->select(mOrderView.currentIndex(), QItemSelectionModel::Select);
    }
}

void OrderWidget::increment() {
    mModel.incrementSelection(mOrderView.selectionModel()->selection());
}

void OrderWidget::decrement() {
    mModel.decrementSelection(mOrderView.selectionModel()->selection());
}

void OrderWidget::set() {
    //bool ok;
    //unsigned id = mSetEdit.text().toUInt(&ok, 16);

    //if (ok) {
        mModel.setSelection(
            mOrderView.selectionModel()->selection(),
            static_cast<uint8_t>(mSetSpin.value())
            );
    //}
}

void OrderWidget::tableViewContextMenu(QPoint pos) {
    mContextMenu.popup(mOrderView.viewport()->mapToGlobal(pos));
}

