
#include "misc/IconManager.hpp"
#include "widgets/docks/TableForm.hpp"

#include <QToolBar>
#include <QVBoxLayout>

#include <QPainter>


TableForm::TableForm(BaseTableModel &model, QString typeName, QWidget *parent) :
    QWidget(parent),
    mModel(model),
    mLayout(),
    mToolbarLayout(),
    mToolbar(),
    mNameEdit(),
    mListView(),
    mContextMenu()
{
    // layout
    mToolbarLayout.addWidget(&mToolbar);
    mToolbarLayout.addWidget(&mNameEdit, 1);
    mLayout.addLayout(&mToolbarLayout);
    mLayout.addWidget(&mListView);
    setLayout(&mLayout);


    mNameEdit.setEnabled(false);

    
    // actions
    mActionAdd.setText(tr("Add"));
    mActionAdd.setIcon(IconManager::getIcon(Icons::itemAdd));
    mActionAdd.setStatusTip(tr("Adds a new %1").arg(typeName));
    mActionRemove.setText(tr("Remove"));
    mActionRemove.setIcon(IconManager::getIcon(Icons::itemRemove));
    mActionRemove.setStatusTip(tr("Removes the current %1").arg(typeName));
    mActionDuplicate.setText(tr("Duplicate"));
    mActionDuplicate.setIcon(IconManager::getIcon(Icons::itemDuplicate));
    mActionDuplicate.setStatusTip(tr("Adds a copy of the current %1").arg(typeName));
    mActionImport.setText(tr("Import"));
    mActionImport.setIcon(IconManager::getIcon(Icons::itemImport));
    mActionImport.setStatusTip(tr("Import %1 from a file").arg(typeName));
    mActionExport.setText(tr("Export"));
    mActionExport.setIcon(IconManager::getIcon(Icons::itemExport));
    mActionExport.setStatusTip(tr("Export %1 to a file").arg(typeName));

    setupMenu(mContextMenu);

    mToolbar.addAction(&mActionAdd);
    mToolbar.addAction(&mActionRemove);
    mToolbar.addAction(&mActionDuplicate);
    mToolbar.addAction(&mActionImport);
    mToolbar.addAction(&mActionExport);
    mToolbar.setIconSize(QSize(16, 16));

    mActionRemove.setEnabled(false);
    mActionDuplicate.setEnabled(false);
    // disable these until we add support for them
    mActionImport.setEnabled(false);
    mActionExport.setEnabled(false);

    mListView.setWrapping(true);
    mListView.setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    mListView.setResizeMode(QListView::Adjust);
    connect(&mListView, &QListView::customContextMenuRequested, this, &TableForm::viewContextMenu);

    mListView.setModel(&mModel);
    auto selectModel = mListView.selectionModel();
    connect(selectModel, &QItemSelectionModel::selectionChanged, this, &TableForm::viewSelectChanged);
    //connect(&mModel, &BaseTableModel::currentIndexChanged, this, &TableForm::modelCurrentChanged);
    connect(&mNameEdit, &QLineEdit::textEdited, this, qOverload<const QString&>(&TableForm::nameEdited));

    // actions
    connect(&mActionAdd, &QAction::triggered, this, &TableForm::add);
    connect(&mActionRemove, &QAction::triggered, this, &TableForm::remove);
    connect(&mActionDuplicate, &QAction::triggered, this, &TableForm::duplicate);

    //connect(&mActionRemove, &QAction::triggered, &mModel, qOverload<>(&BaseTableModel::remove));
    //connect(&mActionDuplicate, &QAction::triggered, &mModel, qOverload<>(&BaseTableModel::duplicate));
    //connect(&mActionEdit, &QAction::triggered, this, &TableForm::showEditor);

    // TODO: connect import action
    // TODO: connect export action

    //connect(&mModel, &BaseTableModel::addEnable, &mActionAdd, &QAction::setEnabled);
    //connect(&mModel, &BaseTableModel::duplicateEnable, &mActionDuplicate, &QAction::setEnabled);
    //connect(&mModel, &BaseTableModel::removeEnable, &mActionRemove, &QAction::setEnabled);


    connect(&mListView, &QListView::doubleClicked, this,
        [this](const QModelIndex &index) {
            (void)index;
            emit showEditor();
        });

}

void TableForm::setupMenu(QMenu &menu) {
    menu.addAction(&mActionAdd);
    menu.addAction(&mActionRemove);
    menu.addAction(&mActionDuplicate);
    menu.addSeparator();
    menu.addAction(&mActionImport);
    menu.addAction(&mActionExport);
}

void TableForm::viewSelectChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    Q_UNUSED(deselected)

    bool hasSelection = selected.size() > 0;

    mNameEdit.setEnabled(hasSelection);
    mActionRemove.setEnabled(hasSelection);
    mActionDuplicate.setEnabled(hasSelection && mModel.canDuplicate());

    if (hasSelection) {
        mNameEdit.setText(mModel.name(selected[0].topLeft().row()));
    } else {
        mNameEdit.clear();
    }
    
}

void TableForm::add() {
    mModel.add();
    if (!mModel.canDuplicate()) {
        mActionAdd.setEnabled(false);
        mActionDuplicate.setEnabled(false);
    }
}

void TableForm::remove() {
    mModel.remove(mListView.currentIndex().row());
    mActionAdd.setEnabled(true);
    mActionDuplicate.setEnabled(mListView.selectionModel()->hasSelection());
}

void TableForm::duplicate() {
    mModel.duplicate(mListView.currentIndex().row());
    if (!mModel.canDuplicate()) {
        mActionAdd.setEnabled(false);
        mActionDuplicate.setEnabled(false);
    }
}


void TableForm::nameEdited(QString const& text) {
    mModel.rename(mListView.currentIndex().row(), text);
}

void TableForm::viewContextMenu(const QPoint &pos) {
    mContextMenu.popup(mListView.viewport()->mapToGlobal(pos));
}

