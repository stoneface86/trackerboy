
#include "widgets/docks/TableForm.hpp"
#include "Tileset.hpp"

#include <QToolBar>
#include <QVBoxLayout>

#include <QPainter>

constexpr int ICON_WIDTH = 16;
constexpr int ICON_HEIGHT = 16;

constexpr int ICON_ADD = 0;
constexpr int ICON_REMOVE = 1;
constexpr int ICON_DUPLICATE = 2;
constexpr int ICON_IMPORT = 3;
constexpr int ICON_EXPORT = 4;
constexpr int ICON_EDIT = 5;


TableForm::TableForm(BaseTableModel &model, QWidget *editor, QString editorShortcut, QString typeName, QWidget *parent) :
    mModel(model),
    mEditor(editor),
    mMenu(new QMenu(this)),
    mListView(new QListView()),
    mNameEdit(new QLineEdit()),
    QWidget(parent)
{
    mNameEdit->setEnabled(false);

    Tileset tileset(QImage(":/icons/tableFormIcons.png"), ICON_WIDTH, ICON_HEIGHT);


    mActAdd = mMenu->addAction(tr("Add"));
    mActAdd->setIcon(tileset.getIcon(ICON_ADD));
    mActAdd->setStatusTip(QString("Adds a new %1").arg(typeName));
    mActRemove = mMenu->addAction(tr("Remove"));
    mActRemove->setIcon(tileset.getIcon(ICON_REMOVE));
    mActRemove->setStatusTip(QString("Removes the current %1").arg(typeName));
    mActDuplicate = mMenu->addAction(tr("Duplicate"));
    mActDuplicate->setIcon(tileset.getIcon(ICON_DUPLICATE));
    mActDuplicate->setStatusTip(QString("Adds a copy of the current %1").arg(typeName));
    mMenu->addSeparator();
    mActImport = mMenu->addAction(tr("Import"));
    mActImport->setIcon(tileset.getIcon(ICON_IMPORT));
    mActImport->setStatusTip(QString("Import %1 from a file").arg(typeName));
    mActExport = mMenu->addAction(tr("Export"));
    mActExport->setIcon(tileset.getIcon(ICON_EXPORT));
    mActExport->setStatusTip(QString("Export %1 to a file").arg(typeName));
    mMenu->addSeparator();
    mActEdit = mMenu->addAction(tr("Edit"));
    mActEdit->setIcon(tileset.getIcon(ICON_EDIT));
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
    connect(mNameEdit, &QLineEdit::textEdited, &mModel, &BaseTableModel::rename);

    // actions
    connect(mActAdd, &QAction::triggered, &mModel, &BaseTableModel::add);
    connect(mActRemove, &QAction::triggered, &mModel, &BaseTableModel::remove);
    connect(mActDuplicate, &QAction::triggered, &mModel, &BaseTableModel::duplicate);

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

