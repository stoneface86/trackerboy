#pragma once

#include "backend.hxx"

#include <QObject>
#include <QUndoCommand>
#include <QUndoGroup>

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
    // Performs an edit via the given QUndoCommand. Ownership of `cmd` is
    // transferred to this document's QUndoGroup.
    //
    void edit(QUndoCommand *cmd);

    //
    // Get a `ViewContext` for read-only access to the document's source data.
    //
    ViewContext view() const;

    //
    // Gets the QUndoGroup in use by this document. Each song has its own
    // QUndoStack. To change the active stack, use `selectSong`.
    //
    QUndoGroup const *undoGroup() const;

    //
    // Select the current song for editing and playback. The `songChanged`
    // signal will be emitted and the song's QUndoStack will be set to the
    // active stack in this document's QUndoGroup.
    //
    void selectSong(int songNo);

    //
    // Gets the current song index that was selected.
    //
    int song() const;

    //
    // Updates all song QUndoStacks after the song list has been changed. Call
    // this after opening a module, creating a new module, or when the user
    // has modified the song list via the Song List Editor.
    //
    void songListChanged();

signals:
    void aboutToSave();
    void reloaded();
    void songChanged(int songNo);
    void modifiedChanged(bool modified);

private:
    NimRef<B::Document> mSource;
    bool mModified;
    QUndoGroup *mUndoGroup;

    struct SongHistory {
        qintptr id; // this is actually the pointer of the song
        QUndoStack *stack;
    };

    QList<SongHistory> initHistoryFromSource();

    QList<SongHistory> mSongHistories;
    int mCurrentSong;
};