
#pragma once

#include "core/Document.hxx"
#include "forms/PersistantDialog.hxx"

#include <QPlainTextEdit>

class CommentsDialog : public PersistantDialog {

    Q_OBJECT

public:

    explicit CommentsDialog(Document *doc, QWidget *parent = nullptr);
    virtual ~CommentsDialog() = default;

private:
    Q_DISABLE_COPY(CommentsDialog)

    void reload();

    void commit();

    void onTextChanged();

    Document *mDocument;
    QPlainTextEdit *mEdit;
    bool mModified;

};
