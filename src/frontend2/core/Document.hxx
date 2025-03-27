#pragma once

#include "backend.hxx"

#include <QObject>

class Document : public QObject {

    Q_OBJECT

public:
    struct EditContext {
        B::Document *backend;
        Document &document;
        bool setModified;

        explicit EditContext(Document &doc, bool setModified = false);
        ~EditContext();
    };

    explicit Document(QObject *parent = nullptr);
    virtual ~Document() = default;

    void setModified();

    EditContext edit(bool setModified = false);

    B::Document* source();

signals:
    void aboutToSave();
    void reloaded();
    void modifiedChanged(bool modified);



private:
    NimRef<B::Document> mSource;
    bool mModified;

};