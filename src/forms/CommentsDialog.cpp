
#include "forms/CommentsDialog.hpp"
#include "utils/connectutils.hpp"

#include <QVBoxLayout>
#include <QPushButton>

CommentsDialog::CommentsDialog(Module &mod, QWidget *parent) :
    PersistantDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    mModule(mod)
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
    lazyconnect(&mod, aboutToSave, this, commit);
    lazyconnect(&mod, reloaded, this, reload);
    lazyconnect(mEdit, textChanged, &mod, makeDirty);

    reload();
}

void CommentsDialog::reload() {
    QSignalBlocker blocker(mEdit);
    mEdit->setPlainText(QString::fromStdString(mModule.data().comments()));
}

void CommentsDialog::commit() {
    auto editor = mModule.permanentEdit();
    mModule.data().setComments(mEdit->toPlainText().toStdString());
}
