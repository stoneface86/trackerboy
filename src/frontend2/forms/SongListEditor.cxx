
#include "forms/SongListEditor.hxx"
#include "utils/connectutils.hxx"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>

#define TU SongListEditorTU
namespace TU {
int selectedRow(QAbstractItemView *view) {
    auto const curr = view->currentIndex();
    if (curr.isValid()) {
        return curr.row();
    } else {
        return -1;
    }
}

void selectRow(QAbstractItemView *view, int row) {
    view->setCurrentIndex(view->model()->index(row, 0));
}

} // namespace TU

SongListEditor::SongListEditor(SongListModel *model, QWidget *parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                          Qt::WindowCloseButtonHint)
    , mModel(new SongListEditorModel(model, this))
    , mView(new QTreeView) {

    auto layout = new QVBoxLayout;

    auto contentLayout = new QHBoxLayout;

    auto buttonLayout = new QVBoxLayout;
    auto buttonNew = new QPushButton(tr("New"));
    auto buttonDup = new QPushButton(tr("Duplicate"));
    auto buttonMoveUp = new QPushButton(tr("Move Up"));
    auto buttonMoveDown = new QPushButton(tr("Move Down"));

    auto dialogButtons =
        new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel |
                             QDialogButtonBox::Reset);

    layout->addLayout(contentLayout, 1);
    contentLayout->addWidget(mView, 1);
    contentLayout->addLayout(buttonLayout);
    buttonLayout->addWidget(buttonNew);
    buttonLayout->addWidget(buttonDup);
    buttonLayout->addWidget(buttonMoveUp);
    buttonLayout->addWidget(buttonMoveDown);
    buttonLayout->addStretch(1);
    layout->addWidget(dialogButtons);
    setLayout(layout);

    mView->setModel(mModel);
    mView->setSelectionMode(QAbstractItemView::SingleSelection);
    mView->setDragEnabled(true);
    mView->viewport()->setAcceptDrops(true);
    mView->setDragDropMode(QAbstractItemView::InternalMove);

    lazyconnect(buttonNew, clicked, mModel, add);
    lazyconnect(buttonDup, clicked, this, duplicate);
    lazyconnect(buttonMoveUp, clicked, this, moveUp);
    lazyconnect(buttonMoveDown, clicked, this, moveDown);
    lazyconnect(dialogButtons, accepted, this, accept);
    lazyconnect(dialogButtons, rejected, this, reject);
    lazyconnect(dialogButtons->button(QDialogButtonBox::Reset), clicked, mModel,
                reset);

    setWindowTitle(tr("Song list"));
    dialogButtons->button(QDialogButtonBox::Save)
        ->setToolTip(tr("Applies all pending changes to the song list"));
    dialogButtons->button(QDialogButtonBox::Cancel)
        ->setToolTip(tr("Cancels all pending changes and closes the dialog"));
    dialogButtons->button(QDialogButtonBox::Reset)
        ->setToolTip(tr("Reverts all pending changes"));

    dumpObjectTree();
}

void SongListEditor::applyChanges(Document &doc) {
    mModel->apply(doc);
}

void SongListEditor::revertChanges() {
    mModel->revert();
}

void SongListEditor::duplicate() {
    auto const row = TU::selectedRow(mView);
    if (row != -1) {
        mModel->duplicate(row);
    }
}

void SongListEditor::moveUp() {
    auto const row = TU::selectedRow(mView);
    if (row != -1) {
        mModel->moveUp(row);
        TU::selectRow(mView, row - 1);
    }
}

void SongListEditor::moveDown() {
    auto const row = TU::selectedRow(mView);
    if (row != -1) {
        mModel->moveDown(row);
        TU::selectRow(mView, row + 1);
    }
}

#undef TU
