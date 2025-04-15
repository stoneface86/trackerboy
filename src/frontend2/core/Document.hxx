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
    virtual ~Document() = default;

    void setModified();

    //
    // Get an `EditContext` for editing the document's source data.
    //
    EditContext edit(bool setModified = false);

    // IO

    //
    // Sets this document to the module stored in the given file.
    // `true` is returned on success. On failure, document is left unchanged.
    //
    bool open(QString const& filename);

    //
    // Saves the document to the previous filename used by open or save.
    // `true` is returned on success.
    //
    bool save();

    //
    // Saves the document to the given filename and remembers it.
    // `true` is returned on success.
    //
    bool save(QString const& filename);

    QString crashSave();

    QString name() const;

    void setName(QString const& name);

    QString path() const;

    void clearPath();

    bool hasFile() const;

    void setAutoBackup(bool autoBackup);

    B::Document* source();
    B::Document const* source() const;

signals:
    void aboutToSave();
    void reloaded();
    void modifiedChanged(bool modified);



private:
    bool doSave(QString const& filename);
    void updateFilename(QString const& filename);


    NimRef<B::Document> mSource;
    bool mModified;

    QString mFilename;
    QString mFilepath;
    bool mAutoBackup;

};