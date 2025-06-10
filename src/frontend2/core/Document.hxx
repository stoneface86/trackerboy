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

        inline B::Document *operator->() { return backend; }
    };

    //
    // An object representing a context for viewing the document's source data.
    //
    struct ViewContext {
        B::Document const *backend;
        Document const &document;

        explicit ViewContext(Document const &doc);
        ~ViewContext();

        inline B::Document const *operator->() { return backend; }
    };

    explicit Document(QObject *parent = nullptr);

    void setModified();

    //
    // Get an `EditContext` for editing the document's source data.
    //
    EditContext edit(bool setModified = false);

    //
    // Get a `ViewContext` for read-only access to the document's source data.
    //
    ViewContext view() const;

signals:
    void aboutToSave();
    void reloaded();
    void modifiedChanged(bool modified);

private:
    NimRef<B::Document> mSource;
    bool mModified;
};