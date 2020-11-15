#include "widgets/docks/OrderWidget.hpp"

#pragma warning(push, 0)
#include "ui_OrderWidget.h"
#pragma warning(pop)

#include <QFontDatabase>
#include <QtDebug>

OrderWidget::OrderWidget(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::OrderWidget()),
    mModel(nullptr),
    mContextMenu(nullptr)
{
    mUi->setupUi(this);
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

}

OrderWidget::~OrderWidget() {
    delete mUi;
}

void OrderWidget::init(OrderModel *model, QMenu *menu) {
    mModel = model;
    mContextMenu = menu;
    mUi->tableView->setModel(model);
    connect(mUi->tableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &OrderWidget::currentChanged);
    mUi->tableView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(mUi->tableView, &QTableView::customContextMenuRequested, this, &OrderWidget::tableViewContextMenu);
    /*mUi->insertButton->setAction(actions.insert);
    mUi->removeButton->setAction(actions.remove);
    mUi->duplicateButton->setAction(actions.duplicate);
    mUi->moveUpButton->setAction(actions.moveUp);
    mUi->moveDownButton->setAction(actions.moveDown);*/

}

void OrderWidget::currentChanged(QModelIndex const &current, QModelIndex const &prev) {
    // TODO: set PatternGrid's cursor to this track
 
}

void OrderWidget::increment() {
    mModel->incrementSelection(mUi->tableView->selectionModel()->selection());
}

void OrderWidget::decrement() {
    mModel->decrementSelection(mUi->tableView->selectionModel()->selection());
}

void OrderWidget::set() {
    bool ok;
    unsigned id = mUi->setLineEdit->text().toUInt(&ok, 16);

    if (ok) {
        mModel->setSelection(
            mUi->tableView->selectionModel()->selection(),
            id
            );
    }
}

void OrderWidget::tableViewContextMenu(QPoint pos) {
    mContextMenu->popup(mUi->tableView->viewport()->mapToGlobal(pos));
}

