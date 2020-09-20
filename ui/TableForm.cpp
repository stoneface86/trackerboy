
#include "TableForm.hpp"

#include <QToolbar>
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

QIcon fromTileset(QImage &tileset, int index) {
    QImage result(QSize(ICON_WIDTH, ICON_HEIGHT), tileset.format());
    
    QIcon icon;

    // normal icon
    {
        result.fill(Qt::transparent);
        QPainter painter(&result);
        painter.setCompositionMode(QPainter::CompositionMode::CompositionMode_SourceOver);
        painter.drawImage(0, 0, tileset, index * ICON_WIDTH, 0, ICON_WIDTH, ICON_HEIGHT);
    }
    icon.addPixmap(QPixmap::fromImage(result), QIcon::Normal);

    // disabled icon
    {
        result.fill(Qt::transparent);
        QPainter painter(&result);
        painter.setCompositionMode(QPainter::CompositionMode::CompositionMode_SourceOver);
        painter.drawImage(0, 0, tileset, index * ICON_WIDTH, ICON_HEIGHT, ICON_WIDTH, ICON_HEIGHT);
    }
    icon.addPixmap(QPixmap::fromImage(result), QIcon::Disabled);

    return icon;
}


TableForm::TableForm(QWidget *parent) :
    mMenu(new QMenu(this)),
    mListView(new QListView()),
    mNameEdit(new QLineEdit()),
    QWidget(parent)
{
    mNameEdit->setPlaceholderText("Name");
    mNameEdit->setEnabled(false);

    QImage tileset(":/icons/tableFormIcons.png");


    mActAdd = mMenu->addAction(tr("Add"));
    mActAdd->setIcon(fromTileset(tileset, ICON_ADD));
    mActRemove = mMenu->addAction(tr("Remove"));
    mActRemove->setIcon(fromTileset(tileset, ICON_REMOVE));
    mActDuplicate = mMenu->addAction(tr("Duplicate"));
    mActDuplicate->setIcon(fromTileset(tileset, ICON_DUPLICATE));
    mMenu->addSeparator();
    mActImport = mMenu->addAction(tr("Import"));
    mActImport->setIcon(fromTileset(tileset, ICON_IMPORT));
    mActExport = mMenu->addAction(tr("Export"));
    mActExport->setIcon(fromTileset(tileset, ICON_EXPORT));
    mMenu->addSeparator();
    mActEdit = mMenu->addAction(tr("Edit"));
    mActEdit->setIcon(fromTileset(tileset, ICON_EDIT));

    auto toolbar = new QToolBar();
    toolbar->addAction(mActAdd);
    toolbar->addAction(mActRemove);
    toolbar->addAction(mActDuplicate);
    //toolbar->addSeparator();
    toolbar->addAction(mActImport);
    toolbar->addAction(mActExport);
    //toolbar->addSeparator();
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

