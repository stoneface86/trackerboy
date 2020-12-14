
#include "misc/IconManager.hpp"
#include "widgets/docks/TableForm.hpp"

#include <QToolBar>
#include <QVBoxLayout>

#include <QPainter>

constexpr int ICON_WIDTH = 16;
constexpr int ICON_HEIGHT = 16;


TableForm::TableForm(BaseTableModel &model, QWidget *editor, QString editorShortcut, QString typeName, QWidget *parent) :
    mModel(model),
    mEditor(editor),
    mMenu(new QMenu(this)),
    mListView(new QListView()),
    mNameEdit(new QLineEdit()),
    QWidget(parent)
{
    mNameEdit->setEnabled(false);

    

    mActAdd = mMenu->addAction(tr("Add"));
    mActAdd->setIcon(IconManager::getIcon(Icons::itemAdd));
    mActAdd->setStatusTip(QString("Adds a new %1").arg(typeName));
    mActRemove = mMenu->addAction(tr("Remove"));
    mActRemove->setIcon(IconManager::getIcon(Icons::itemRemove));
    mActRemove->setStatusTip(QString("Removes the current %1").arg(typeName));
    mActDuplicate = mMenu->addAction(tr("Duplicate"));
    mActDuplicate->setIcon(IconManager::getIcon(Icons::itemDuplicate));
    mActDuplicate->setStatusTip(QString("Adds a copy of the current %1").arg(typeName));
    mMenu->addSeparator();
    mActImport = mMenu->addAction(tr("Import"));
    mActImport->setIcon(IconManager::getIcon(Icons::itemImport));
    mActImport->setStatusTip(QString("Import %1 from a file").arg(typeName));
    mActExport = mMenu->addAction(tr("Export"));
    mActExport->setIcon(IconManager::getIcon(Icons::itemExport));
    mActExport->setStatusTip(QString("Export %1 to a file").arg(typeName));
    mMenu->addSeparator();
    mActEdit = mMenu->addAction(tr("Edit"));
    mActEdit->setIcon(IconManager::getIcon(Icons::itemEdit));
    mActEdit->setStatusTip(QString("Edit the current %1").arg(typeName));

    auto toolbar = new QToolBar();
    toolbar->addAction(mActAdd);
    toolbar->addAction(mActRemove);
    toolbar->addAction(mActDuplicate);
    toolbar->addAction(mActImport);
    toolbar->addAction(mActExport);
    toolbar->addAction(mActEdit);
    toolbar->setIconSize(QSize(ICON_WIDTH, ICON_HEIGHT));


    auto layout = new QVBoxLayout();
    auto toolbarLayout = new QHBoxLayout();
    toolbarLayout->addWidget(toolbar, 0);
    toolbarLayout->addWidget(mNameEdit, 1);
    layout->addLayout(toolbarLayout);
    layout->addWidget(mListView);
    setLayout(layout);

    mActRemove->setEnabled(false);
    mActDuplicate->setEnabled(false);
    mActEdit->setEnabled(false);
    // disable these until we add support for them
    mActImport->setEnabled(false);
    mActExport->setEnabled(false);

    mListView->setWrapping(true);
    mListView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(mListView, &QListView::customContextMenuRequested, this, &TableForm::viewContextMenu);

    mModel.setActions(mActAdd, mActRemove, mActDuplicate, mActEdit);
    mListView->setModel(&mModel);
    auto selectModel = mListView->selectionModel();
    connect(selectModel, &QItemSelectionModel::currentChanged, this, &TableForm::viewCurrentChanged);
    connect(&mModel, &BaseTableModel::currentIndexChanged, this, &TableForm::modelCurrentChanged);
    connect(mNameEdit, &QLineEdit::textEdited, &mModel, qOverload<const QString&>(&BaseTableModel::rename));

    // actions
    connect(mActAdd, &QAction::triggered, &mModel, &BaseTableModel::add);
    connect(mActRemove, &QAction::triggered, &mModel, qOverload<>(&BaseTableModel::remove));
    connect(mActDuplicate, &QAction::triggered, &mModel, qOverload<>(&BaseTableModel::duplicate));

    // TODO: connect import action
    // TODO: connect export action

    mEditor = editor;
    connect(mActEdit, &QAction::triggered, editor, &QWidget::show);
    connect(mListView, &QListView::doubleClicked, this,
        [this](const QModelIndex &index) {
            (void)index;
            mEditor->show();
        });

    mActEdit->setShortcut(QKeySequence(editorShortcut));

    

}

QMenu* TableForm::menu() const {
    return mMenu;
}

void TableForm::modelCurrentChanged(int index) {
    if (index == -1) {
        mNameEdit->setEnabled(false);
        mNameEdit->setText("");
        mEditor->setVisible(false);
    } else {
        mNameEdit->setEnabled(true);
        mNameEdit->setText(mModel.name());
    }
    mListView->setCurrentIndex(mModel.index(index));
}

void TableForm::viewCurrentChanged(const QModelIndex &current, const QModelIndex &prev) {
    (void)prev; // we don't care about the previous index
    mModel.select(current);
}

void TableForm::viewContextMenu(const QPoint &pos) {
    mMenu->popup(mListView->viewport()->mapToGlobal(pos));
}

