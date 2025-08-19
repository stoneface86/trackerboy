
#include "forms/CommentsDialog.hxx"
#include "utils/backendutils.hxx"
#include "utils/connectutils.hxx"

#include <QPushButton>
#include <QVBoxLayout>

CommentsDialog::CommentsDialog(Document *doc, QWidget *parent)
    : PersistantDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                                   Qt::WindowCloseButtonHint)
    , _document(doc)
    , _edit()
    , _modified(false) {
    setWindowTitle(tr("Comments"));

    auto layout = new QVBoxLayout;
    _edit = new QPlainTextEdit;

    auto closeButton = new QPushButton(tr("Close"));
    closeButton->setDefault(true);

    layout->addWidget(_edit, 1);
    layout->addWidget(closeButton, 0, Qt::AlignHCenter);
    setLayout(layout);

    lazyconnect(closeButton, clicked, this, accept);
    lazyconnect(doc, aboutToSave, this, commit);
    lazyconnect(doc, reloaded, this, reload);
    lazyconnect(_edit, textChanged, this, onTextChanged);

    reload();
}

void CommentsDialog::reload() {
    QSignalBlocker blocker(_edit);
    _edit->setPlainText(toQString(_document->view()->mod.comments()));
    _modified = false;
}

void CommentsDialog::commit() {
    if (_modified) {
        auto str = toNimString(_edit->toPlainText());
        _document->edit()->mod.setComments(str.s);
        _modified = false;
    }
}

void CommentsDialog::onTextChanged() {
    _modified = true;
    _document->setModified();
}