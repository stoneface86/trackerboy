
#include "TableForm.hpp"

#include <QToolbar>
#include <QVBoxLayout>


TableForm::TableForm(QWidget *parent) :
    mMenu(new QMenu(this)),
    mListView(new QListView()),
    mNameEdit(new QLineEdit()),
    QWidget(parent)
{
    mNameEdit->setPlaceholderText("Name");
    mNameEdit->setEnabled(false);

    mActAdd = mMenu->addAction(tr("Add"));
    mActAdd->setIcon(QIcon(":/icons/tableAddItem.png"));
    mActRemove = mMenu->addAction(tr("Remove"));
    mActRemove->setIcon(QIcon(":/icons/tableRemoveItem.png"));
    mActDuplicate = mMenu->addAction(tr("Duplicate"));
    mActDuplicate->setIcon(QIcon(":/icons/tableDuplicateItem.png"));
    mMenu->addSeparator();
    mActImport = mMenu->addAction(tr("Import"));
    mActImport->setIcon(QIcon(":/icons/tableImport.png"));
    mActExport = mMenu->addAction(tr("Export"));
    mActExport->setIcon(QIcon(":/icons/tableExport.png"));
    mMenu->addSeparator();
    mActEdit = mMenu->addAction(tr("Edit"));
    mActEdit->setIcon(QIcon(":/icons/tableEditItem.png"));

    auto toolbar = new QToolBar();
    toolbar->addAction(mActAdd);
    toolbar->addAction(mActRemove);
    toolbar->addAction(mActDuplicate);
    toolbar->addSeparator();
    toolbar->addAction(mActImport);
    toolbar->addAction(mActExport);
    toolbar->addSeparator();
    toolbar->addAction(mActEdit);
    toolbar->setIconSize(QSize(16, 16));


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


}

QMenu* TableForm::menu() const {
    return mMenu;
}

void TableForm::init(BaseTableModel *model, QWidget *editor, QString editorShortcut) {
    mModel = model;
    model->setActions(mActAdd, mActRemove, mActDuplicate, mActEdit);
    mListView->setModel(model);
    auto selectModel = mListView->selectionModel();
    connect(selectModel, &QItemSelectionModel::currentChanged, this, &TableForm::viewCurrentChanged);
    connect(model, &BaseTableModel::currentIndexChanged, this, &TableForm::modelCurrentChanged);
    connect(mNameEdit, &QLineEdit::textEdited, model, &BaseTableModel::rename);

    // actions
    connect(mActAdd, &QAction::triggered, model, &BaseTableModel::add);
    connect(mActRemove, &QAction::triggered, model, &BaseTableModel::remove);
    connect(mActDuplicate, &QAction::triggered, model, &BaseTableModel::duplicate);

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

void TableForm::modelCurrentChanged(int index) {
    if (index == -1) {
        mNameEdit->setEnabled(false);
        mNameEdit->setText("");
        mEditor->setVisible(false);
    } else {
        mNameEdit->setEnabled(true);
        mNameEdit->setText(mModel->name());
    }
    mListView->setCurrentIndex(mModel->index(index));
}

void TableForm::viewCurrentChanged(const QModelIndex &current, const QModelIndex &prev) {
    (void)prev; // we don't care about the previous index
    mModel->select(current);
}

void TableForm::viewContextMenu(const QPoint &pos) {
    mMenu->popup(mListView->viewport()->mapToGlobal(pos));
}

