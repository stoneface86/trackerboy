
#include "widgets/docks/TableDock.hpp"

#include "utils/connectutils.hpp"
#include "utils/IconLocator.hpp"

#include <QBoxLayout>
#include <QToolBar>
#include <QtDebug>

TableDock::TableDock(
    BaseTableModel &model,
    QKeySequence editorShortcut,
    QString const& typeName,
    QWidget *parent
) :
    QWidget(parent),
    mModel(model),
    mActions(),
    mView(nullptr),
    mSelectedItem(-1)
{

    auto layout = new QBoxLayout(QBoxLayout::TopToBottom);
    auto toolbar = new QToolBar;
    mView = new QListView;

    layout->addWidget(toolbar);
    layout->addWidget(mView, 1);
    setLayout(layout);

    mView->setModel(&model);
    mView->setWrapping(true);

    toolbar->setIconSize(IconLocator::size());

    QAction *act;

    act = toolbar->addAction(tr("Add"));
    act->setIcon(IconLocator::get(Icons::itemAdd));
    act->setStatusTip(tr("Adds a new %1").arg(typeName));
    connectActionToThis(act, add);
    mActions.add = act;
    
    act = toolbar->addAction(tr("Remove"));
    act->setIcon(IconLocator::get(Icons::itemRemove));
    act->setStatusTip(tr("Removes the current %1").arg(typeName));
    connectActionToThis(act, remove);
    mActions.remove = act;

    act = toolbar->addAction(tr("Duplicate"));
    act->setIcon(IconLocator::get(Icons::itemDuplicate));
    act->setStatusTip(tr("Adds a copy of the current %1").arg(typeName));
    connectActionToThis(act, duplicate);
    mActions.duplicate = act;
    
    act = toolbar->addAction(tr("Import"));
    act->setIcon(IconLocator::get(Icons::itemImport));
    act->setStatusTip(tr("Import %1 from a file").arg(typeName));
    act->setEnabled(false); // disable for now
    mActions.importFile = act;
    
    act = toolbar->addAction(tr("Export"));
    act->setIcon(IconLocator::get(Icons::itemExport));
    act->setStatusTip(tr("Export %1 to a file").arg(typeName));
    act->setEnabled(false);
    mActions.exportFile = act;

    act = toolbar->addAction(tr("Edit"));
    act->setIcon(IconLocator::get(Icons::itemEdit));
    act->setStatusTip(tr("Edit the current %1").arg(typeName));
    act->setShortcut(editorShortcut);
    connect(act, &QAction::triggered, this,
        [this]() {
            emit edit(mSelectedItem);
        });
    mActions.edit = act;

    updateActions();

    // act = toolbar->addAction(tr("Vertical"));
    // act->setCheckable(true);
    // act->setChecked(true);
    // connect(act, &QAction::toggled, this,
    //     [this, layout, toolbar](bool checked) {
    //         if (checked) {
    //             layout->setDirection(QBoxLayout::TopToBottom);
    //             toolbar->setOrientation(Qt::Horizontal);
    //         } else {
    //             layout->setDirection(QBoxLayout::LeftToRight);
    //             toolbar->setOrientation(Qt::Vertical);
    //         }
    //     });

    auto selectionModel = mView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this,
        [this](QItemSelection const& selected) {

            // single-selection mode
            // possible reasons for this signal:
            // 1. user selected when there was no selection
            // 2. user selected a new item, implicit deselect of the old item
            // 3. user deselected item explicitly

            if (selected.isEmpty()) {
                // [3] deselection
                setSelected(-1);
            } else {
                // [1], [2] selection made (we don't care about the previous selection)
                setSelected(selected.indexes().first().row());
            }
        });

    connect(&model, &BaseTableModel::modelReset, this,
        [this]() {
            // the selectionChanged signal is not fired when the model resets
            setSelected(-1);
        });

    connect(mView, &QListView::doubleClicked, this,
        [this](QModelIndex const& index) {
            emit edit(index.row());
        });

}

TableActions const& TableDock::tableActions() const {
    return mActions;
}

void TableDock::add() {
    mModel.add();
    if (!mModel.canAdd()) {
        mActions.add->setEnabled(false);
        mActions.duplicate->setEnabled(false);
    }
    updateActions();
}

void TableDock::remove() {
    mModel.remove(mSelectedItem);
    if (mSelectedItem == mModel.rowCount()) {
        --mSelectedItem;
        emit selectedItemChanged(mSelectedItem);
    }
    updateActions();
}

void TableDock::duplicate() {
    mModel.duplicate(mSelectedItem);
    updateActions();
}

int TableDock::selectedItem() const {
    return mSelectedItem;
}

void TableDock::setSelectedItem(int item) {
    mView->selectionModel()->select(mModel.index(item), QItemSelectionModel::ClearAndSelect);
}

void TableDock::setSelected(int index) {
    if (mSelectedItem != index) {
        bool hadItem = mSelectedItem != -1;
        mSelectedItem = index;
        emit selectedItemChanged(index);
        if ((index != -1) != hadItem) {
            updateActions();
        }
    }
}

void TableDock::updateActions() {
    auto const canAdd = mModel.canAdd();
    auto const hasSelection = mSelectedItem != -1;
    mActions.add->setEnabled(canAdd);
    mActions.remove->setEnabled(hasSelection);
    mActions.duplicate->setEnabled(canAdd && hasSelection);
    mActions.edit->setEnabled(hasSelection);
}
