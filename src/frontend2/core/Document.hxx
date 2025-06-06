#pragma once

#include "backend.hxx"

#include <QObject>

class Document : public QObject {

    Q_OBJECT

public:
    //
    // An object representing a context for editing the document's source data.
    // The document is locked for the lifetime of this object. Edits should
    // only be made when you have an EditContext object.
    //
    struct EditContext {
        B::Document *backend;
        Document &document;
        bool setModified;

        explicit EditContext(Document &doc, bool setModified = false);
        ~EditContext();
    };

    explicit Document(QObject *parent = nullptr);

    void setModified();

    //
    // Get an `EditContext` for editing the document's source data.
    //
    EditContext edit(bool setModified = false);

    B::Document *source();
    B::Document const *source() const;

signals:
    void aboutToSave();
    void reloaded();
    void modifiedChanged(bool modified);

private:
    NimRef<B::Document> mSource;
    bool mModified;
};