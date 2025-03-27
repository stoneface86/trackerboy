
#include "forms/CommentsDialog.hxx"
#include "utils/connectutils.hxx"
#include "utils/backendutils.hxx"

#include <QVBoxLayout>
#include <QPushButton>

CommentsDialog::CommentsDialog(Document *doc, QWidget *parent) :
    PersistantDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    mDocument(doc),
    mEdit()
{
    setWindowTitle(tr("Comments"));

    auto layout = new QVBoxLayout;
    mEdit = new QPlainTextEdit;

    auto closeButton = new QPushButton(tr("Close"));
    closeButton->setDefault(true);

    layout->addWidget(mEdit, 1);
    layout->addWidget(closeButton, 0, Qt::AlignHCenter);
    setLayout(layout);

    lazyconnect(closeButton, clicked, this, accept);
    lazyconnect(doc, aboutToSave, this, commit);
    lazyconnect(doc, reloaded, this, reload);
    lazyconnect(mEdit, textChanged, this, onTextChanged);

    reload();
}

void CommentsDialog::reload() {
    QSignalBlocker blocker(mEdit);
    mEdit->setPlainText(toString(B::comments(mDocument->source())));
    mModified = false;
}

void CommentsDialog::commit() {
    if (mModified) {
        auto temp = toUtf8(mEdit->toPlainText());
        B::setComments(mDocument->source(), temp.slice);
        mModified = false;
    }
}

void CommentsDialog::onTextChanged() {
    mModified = true;
    mDocument->setModified();
}