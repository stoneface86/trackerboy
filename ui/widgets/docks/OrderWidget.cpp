
#include "widgets/docks/OrderWidget.hpp"

#include "misc/IconManager.hpp"

#include <QFontDatabase>
#include <QHeaderView>
#include <QtDebug>


OrderWidget::OrderWidget(OrderModel &model, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mContextMenu(nullptr),
    mActions(),
    mLayout(QBoxLayout::TopToBottom),
    mLayoutOperations(QBoxLayout::LeftToRight),
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

    // actions
    mActions.insert = new QAction(IconManager::getIcon(Icons::itemAdd), tr("&Insert order"), this);
    connect(mActions.insert, &QAction::triggered, &model, &OrderModel::insert);
    connect(&model, &OrderModel::canInsert, mActions.insert, &QAction::setEnabled);

    mActions.remove = new QAction(IconManager::getIcon(Icons::itemRemove), tr("&Remove order"), this);
    mActions.remove->setEnabled(false);
    connect(mActions.remove, &QAction::triggered, &model, &OrderModel::remove);
    connect(&model, &OrderModel::canRemove, mActions.remove, &QAction::setEnabled);

    mActions.duplicate = new QAction(IconManager::getIcon(Icons::itemDuplicate), tr("&Duplicate order"), this);
    connect(mActions.duplicate, &QAction::triggered, &model, &OrderModel::duplicate);
    connect(&model, &OrderModel::canInsert, mActions.duplicate, &QAction::setEnabled);

    mActions.moveUp = new QAction(IconManager::getIcon(Icons::moveUp), tr("Move order &up"), this);
    mActions.moveUp->setEnabled(false);
    connect(mActions.moveUp, &QAction::triggered, &model, &OrderModel::moveUp);
    connect(&model, &OrderModel::canMoveUp, mActions.moveUp, &QAction::setEnabled);

    mActions.moveDown = new QAction(IconManager::getIcon(Icons::moveDown), tr("Move order &down"), this);
    mActions.moveDown->setEnabled(false);
    connect(mActions.moveDown, &QAction::triggered, &model, &OrderModel::moveDown);
    connect(&model, &OrderModel::canMoveDown, mActions.moveDown, &QAction::setEnabled);

    // menu
    mContextMenu = createMenu(this);


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

QMenu* OrderWidget::createMenu(QWidget *parent) {
    auto menu = new QMenu(parent);
    menu->addAction(mActions.insert);
    menu->addAction(mActions.remove);
    menu->addAction(mActions.duplicate);
    menu->addSeparator();
    menu->addAction(mActions.moveUp);
    menu->addAction(mActions.moveDown);
    return menu;
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
    mContextMenu->popup(mOrderView.viewport()->mapToGlobal(pos));
}

