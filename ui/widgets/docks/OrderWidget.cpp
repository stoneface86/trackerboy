
#include "widgets/docks/OrderWidget.hpp"

#include "misc/IconManager.hpp"

#include <QFontDatabase>
#include <QHeaderView>
#include <QtDebug>


OrderWidget::OrderWidget(OrderModel &model, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mContextMenu(),
    mActions(),
    mLayout(),
    mLayoutOperations(),
    mIncrementButton(QStringLiteral("+")),
    mDecrementButton(QStringLiteral("-")),
    mSetEdit(QStringLiteral("00")),
    mSetButton(tr("Set")),
    mOrderView()
{

    // layout
    mLayoutOperations.addWidget(&mIncrementButton);
    mLayoutOperations.addWidget(&mDecrementButton);
    mLayoutOperations.addStretch(1);
    mLayoutOperations.addWidget(&mSetEdit);
    mLayoutOperations.addWidget(&mSetButton);

    mLayout.addLayout(&mLayoutOperations);
    mLayout.addWidget(&mOrderView, 1);
    setLayout(&mLayout);
    
    // settings
    mIncrementButton.setMaximumWidth(32);
    mDecrementButton.setMaximumWidth(32);

    mSetEdit.setInputMask(QStringLiteral("HH"));
    mSetEdit.setMaxLength(2);
    
    mActions.remove.setEnabled(false);
    mActions.moveUp.setEnabled(false);
    mActions.moveDown.setEnabled(false);

    // actions
    mActions.add.setText(tr("&Insert order row"));
    mActions.add.setStatusTip(tr("Inserts a new order at the current pattern"));
    connect(&mActions.add, &QAction::triggered, &model, &OrderModel::insert);
    connect(&model, &OrderModel::canInsert, &mActions.add, &QAction::setEnabled);

    mActions.remove.setText(tr("&Remove order row"));
    mActions.remove.setStatusTip(tr("Removes the order at the current pattern"));
    connect(&mActions.remove, &QAction::triggered, &model, &OrderModel::remove);
    connect(&model, &OrderModel::canRemove, &mActions.remove, &QAction::setEnabled);

    mActions.duplicate.setText(tr("&Duplicate order row"));
    mActions.duplicate.setStatusTip(tr("Duplicates the order at the current pattern"));
    connect(&mActions.duplicate, &QAction::triggered, &model, &OrderModel::duplicate);
    connect(&model, &OrderModel::canInsert, &mActions.duplicate, &QAction::setEnabled);

    mActions.moveUp.setText(tr("Move order &up"));
    mActions.moveUp.setStatusTip(tr("Moves the order up 1"));
    connect(&mActions.moveUp, &QAction::triggered, &model, &OrderModel::moveUp);
    connect(&model, &OrderModel::canMoveUp, &mActions.moveUp, &QAction::setEnabled);

    mActions.moveDown.setText(tr("Move order dow&n"));
    mActions.moveDown.setStatusTip(tr("Moves the order down 1"));
    connect(&mActions.moveDown, &QAction::triggered, &model, &OrderModel::moveDown);
    connect(&model, &OrderModel::canMoveDown, &mActions.moveDown, &QAction::setEnabled);

    // menu
    setupMenu(mContextMenu);

    mOrderView.setModel(&model);

    auto selectionModel = mOrderView.selectionModel();
    connect(selectionModel, &QItemSelectionModel::currentChanged, this, &OrderWidget::currentChanged);
    connect(&model, &OrderModel::currentIndexChanged, this,
        [this](const QModelIndex &index) {
            mOrderView.selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
        });
    
    mOrderView.setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(&mOrderView, &QTableView::customContextMenuRequested, this, &OrderWidget::tableViewContextMenu);
    
    auto headerView = mOrderView.horizontalHeader();
    headerView->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    // todo: use the same font as the PatternGrid
    mOrderView.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    auto verticalHeader = mOrderView.verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    verticalHeader->setMinimumSectionSize(-1);
    verticalHeader->setDefaultSectionSize(verticalHeader->minimumSectionSize());

    connect(&mIncrementButton, &QPushButton::clicked, this, &OrderWidget::increment);
    connect(&mDecrementButton, &QPushButton::clicked, this, &OrderWidget::decrement);
    connect(&mSetButton, &QPushButton::clicked, this, &OrderWidget::set);

}

OrderWidget::~OrderWidget() {
}

void OrderWidget::setupMenu(QMenu &menu) {
    menu.addAction(&mActions.add);
    menu.addAction(&mActions.remove);
    menu.addAction(&mActions.duplicate);
    menu.addSeparator();
    menu.addAction(&mActions.moveUp);
    menu.addAction(&mActions.moveDown);
}

void OrderWidget::setupToolbar(QToolBar &toolbar) {
    toolbar.addAction(&mActions.add);
    toolbar.addAction(&mActions.remove);
    toolbar.addAction(&mActions.duplicate);
    toolbar.addAction(&mActions.moveUp);
    toolbar.addAction(&mActions.moveDown);
}

void OrderWidget::currentChanged(QModelIndex const &current, QModelIndex const &prev) {
    Q_UNUSED(prev);

    mModel.select(current.row(), current.column());
 
}

void OrderWidget::increment() {
    mModel.incrementSelection(mOrderView.selectionModel()->selection());
}

void OrderWidget::decrement() {
    mModel.decrementSelection(mOrderView.selectionModel()->selection());
}

void OrderWidget::set() {
    bool ok;
    unsigned id = mSetEdit.text().toUInt(&ok, 16);

    if (ok) {
        mModel.setSelection(
            mOrderView.selectionModel()->selection(),
            static_cast<uint8_t>(id)
            );
    }
}

void OrderWidget::tableViewContextMenu(QPoint pos) {
    mContextMenu.popup(mOrderView.viewport()->mapToGlobal(pos));
}

