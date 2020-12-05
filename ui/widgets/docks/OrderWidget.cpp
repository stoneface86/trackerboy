#include "widgets/docks/OrderWidget.hpp"

#pragma warning(push, 0)
#include "ui_OrderWidget.h"
#pragma warning(pop)

#include <QFontDatabase>
#include <QtDebug>

OrderWidget::OrderWidget(OrderModel &model, QMenu *menu, QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::OrderWidget()),
    mModel(model),
    mContextMenu(new QMenu(this))
{
    mUi->setupUi(this);

    mUi->tableView->setModel(&model);

    auto selectionModel = mUi->tableView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::currentChanged, this, &OrderWidget::currentChanged);
    connect(&model, &OrderModel::currentIndexChanged, this,
        [this](const QModelIndex &index) {
            mUi->tableView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
        });
    
    mUi->tableView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(mUi->tableView, &QTableView::customContextMenuRequested, this, &OrderWidget::tableViewContextMenu);
    
    auto headerView = mUi->tableView->horizontalHeader();
    headerView->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);

    

    // todo: use the same font as the PatternGrid
    mUi->tableView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    auto verticalHeader = mUi->tableView->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    verticalHeader->setMinimumSectionSize(-1);
    verticalHeader->setDefaultSectionSize(verticalHeader->minimumSectionSize());

    connect(mUi->incrementButton, &QPushButton::released, this, &OrderWidget::increment);
    connect(mUi->decrementButton, &QPushButton::released, this, &OrderWidget::decrement);
    connect(mUi->setButton, &QPushButton::released, this, &OrderWidget::set);

    for (auto action : menu->actions()) {
        mContextMenu->addAction(action);
    }

}

OrderWidget::~OrderWidget() {
    delete mUi;
}

void OrderWidget::currentChanged(QModelIndex const &current, QModelIndex const &prev) {
    Q_UNUSED(prev);
    mModel.select(current.row(), current.column());
 
}

void OrderWidget::increment() {
    mModel.incrementSelection(mUi->tableView->selectionModel()->selection());
}

void OrderWidget::decrement() {
    mModel.decrementSelection(mUi->tableView->selectionModel()->selection());
}

void OrderWidget::set() {
    bool ok;
    unsigned id = mUi->setLineEdit->text().toUInt(&ok, 16);

    if (ok) {
        mModel.setSelection(
            mUi->tableView->selectionModel()->selection(),
            static_cast<uint8_t>(id)
            );
    }
}

void OrderWidget::tableViewContextMenu(QPoint pos) {
    mContextMenu->popup(mUi->tableView->viewport()->mapToGlobal(pos));
}

