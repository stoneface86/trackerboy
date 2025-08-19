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
    // Gets a QString containing the default song name to be used for new songs.
    //
    QString defaultSongName() const;

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
    // Applies the given list of changes to the document's song list.
    //
    void changeSongList(B::SongListChanges const &changes);

signals:
    //
    // Signal emitted before saving the module to file, models should use this
    // signal to commit any pending changes to the module. Common example is
    // models that keep QString versions of names cached.
    //
    void aboutToSave();

    //
    // The module has been loaded from a file, or a new module was created.
    // `newModule` is `true` if the document is a new module, and `false` if
    // an existing one was loaded.
    //
    void reloaded(bool newModule);

    //
    // The current song selected for editing has changed. This signal is always
    // emitted after the [reloaded] signal, so models should not respond to
    // both.
    //
    void songChanged(int songNo);

    //
    // Signal that is emitted when a permanent change has been made to the
    // document, or if a previous change was reset due to the document being
    // saved or reloaded.
    //
    void modifiedChanged(bool modified);

private:
    struct SongHistory {
        qintptr id; // this is actually the pointer of the song
        QUndoStack *stack;
    };

    QList<SongHistory> initHistoryFromSource();
    void selectSongImpl(int song);

    NimRef<B::Document> _source;
    bool _modified;
    QUndoGroup *_undoGroup;
    QList<SongHistory> _songHistories;
    int _currentSong;
};