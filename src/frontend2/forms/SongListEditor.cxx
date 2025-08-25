
#include "forms/SongListEditor.hxx"
#include "core/settings.hxx"
#include "utils/aliases.hxx"
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

static strlit cGroup = "SongListEditor";

} // namespace TU

SongListEditor::SongListEditor(NameListModel *model, QWidget *parent)
    : PersistantDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                                   Qt::WindowCloseButtonHint)
    , _model(new SongListEditorModel(model, this))
    , _view(new QTreeView) {

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
    contentLayout->addWidget(_view, 1);
    contentLayout->addLayout(buttonLayout);
    buttonLayout->addWidget(buttonNew);
    buttonLayout->addWidget(buttonDup);
    buttonLayout->addWidget(buttonMoveUp);
    buttonLayout->addWidget(buttonMoveDown);
    buttonLayout->addStretch(1);
    layout->addWidget(dialogButtons);
    setLayout(layout);

    _view->setModel(_model);
    _view->setSelectionMode(QAbstractItemView::SingleSelection);
    _view->setDragEnabled(true);
    _view->viewport()->setAcceptDrops(true);
    _view->setDragDropMode(QAbstractItemView::InternalMove);

    lazyconnect(buttonNew, clicked, _model, add);
    lazyconnect(buttonDup, clicked, this, duplicate);
    lazyconnect(buttonMoveUp, clicked, this, moveUp);
    lazyconnect(buttonMoveDown, clicked, this, moveDown);
    lazyconnect(dialogButtons, accepted, this, accept);
    lazyconnect(dialogButtons, rejected, this, reject);
    lazyconnect(dialogButtons->button(QDialogButtonBox::Reset), clicked, _model,
                reset);

    setWindowTitle(tr("Song List"));
    dialogButtons->button(QDialogButtonBox::Save)
        ->setToolTip(tr("Applies all pending changes to the song list"));
    dialogButtons->button(QDialogButtonBox::Cancel)
        ->setToolTip(tr("Cancels all pending changes and closes the dialog"));
    dialogButtons->button(QDialogButtonBox::Reset)
        ->setToolTip(tr("Reverts all pending changes"));

    // restore previous size
    Settings s(SettingsState, TU::cGroup);
    auto size = s.value(lit::size);
    if (!size.isNull()) {
        resize(size.toSize());
    }
}

void SongListEditor::closeEvent(QCloseEvent *evt) {
    Settings s(SettingsState, TU::cGroup);
    s.setValue(lit::size, size());
    PersistantDialog::closeEvent(evt);
}

void SongListEditor::applyChanges(Document &doc) {
    _model->apply(doc);
}

void SongListEditor::revertChanges() {
    _model->revert();
}

void SongListEditor::duplicate() {
    auto const row = TU::selectedRow(_view);
    if (row != -1) {
        _model->duplicate(row);
    }
}

void SongListEditor::moveUp() {
    auto const row = TU::selectedRow(_view);
    if (row != -1) {
        _model->moveUp(row);
        TU::selectRow(_view, row - 1);
    }
}

void SongListEditor::moveDown() {
    auto const row = TU::selectedRow(_view);
    if (row != -1) {
        _model->moveDown(row);
        TU::selectRow(_view, row + 1);
    }
}

#undef TU
