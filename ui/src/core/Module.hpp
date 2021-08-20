
#pragma once

#include "trackerboy/data/Module.hpp"
#include "trackerboy/data/Song.hpp"

#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QUndoGroup>
#include <QUndoStack>
#include <QVector>

//
// Container class for a trackerboy::Module. Also contains a QMutex and
// QUndoStacks for editing. Model classes edit the contained module.
//
class Module : public QObject {

    Q_OBJECT

public:

    //
    // Editor is just a QMutexLocker subclass. This
    // context is used for edits that can be undone, by using a QUndoCommand
    // subclass.
    //
    class Editor : public QMutexLocker {

        friend class Module;

        Editor(Module &module);
    };

    //
    // Extends the Editor, by setting the permanent dirty
    // flag. This editor should be used for edits that cannot be undone
    //
    class PermanentEditor : public Editor {
    public:

        ~PermanentEditor();

    private:
        friend class Module;

        PermanentEditor(Module &module);

        Module &mModule;

    };

    explicit Module(QObject *parent = nullptr);
    virtual ~Module() = default;

    //
    // Clears all data within the module and returns it to its default
    // state. The module is also reset.
    //
    void clear();

    trackerboy::Module const& data() const;
    trackerboy::Module& data();

    //
    // Gets the current song for editing. Since trackerboy modules can never
    // have 0 songs, nullptr is never returned.
    //
    trackerboy::Song* song();

    trackerboy::Song const* song() const;

    //
    // Gets shared ownership of the current song.
    //
    std::shared_ptr<trackerboy::Song> songShared();

    bool isModified() const;

    QMutex& mutex();

    QUndoGroup* undoGroup();

    QUndoStack* undoStack();

    //
    // Reset the module. All undo stacks are deleted and the module is cleaned.
    // The reloaded signal is then emitted. This method is called when the
    // module is cleared (File > New) or a module is loaded (File > Open).
    //
    void reset();

    //
    // Adds a QUndoStack for a song. Called by SongListModel when adding/duplicating
    // a song
    //
    void addSong();

    //
    // Removes a song's QUndoStack. Called by SongListModel when removing a song
    //
    void removeSong(int index);

    //
    // Sets the current song for editing. The song's QUndoStack becomes the
    // active stack for this class's QUndoGroup. The songChanged signal is
    // also emitted, informing models of the change.
    //
    void setSong(int index);

    // Editing ---------------------------------------------------------------

    //
    // Begins an edit operation. The module's mutex is locked and unlocked when
    // the returned editor is destructed.
    //
    Editor edit();

    //
    // Same as edit, but sets the permanent dirty flag on destruction. Edits to
    // the document that cannot be undone should use this context.
    //
    PermanentEditor permanentEdit();

    //
    // Sets the permanent dirty flag. 
    //
    void makeDirty();

    //
    // Clears the permanent dirty flag and cleans the undo stack. Call this when
    // saving the module to disk.
    //
    void clean();

signals:
    //
    // emitted when the clean state or modified state of the module changes.
    //
    void modifiedChanged(bool modified);

    //
    // Emitted when the module has been reset. This occurs when:
    //  * User created a new document, the module was cleared
    //  * User opened a module: The module data was overwritten with the loaded module
    //
    // Connect this signal to models so that they can update to the new module data.
    //
    void reloaded();

    //
    // Emitted when the current song has changed. Also emitted when the module
    // has been reset.
    //
    void songChanged();

private:

    //
    // initialize the undo stack vector to the given size. Existing stacks are
    // reused, their contents are just cleared.
    //
    void resizeUndoStacks(int size);

    Q_DISABLE_COPY(Module)

    trackerboy::Module mModule;

    QMutex mMutex;
    QUndoGroup *mUndoGroup;

    QVector<QUndoStack*> mSongUndoStacks;

    std::shared_ptr<trackerboy::Song> mSong;

    // permanent dirty flag. Not all edits to the document can be undone. When such
    // edit occurs, this flag is set to true. It is reset when the document is
    // saved or when the document is reset or loaded from disk.
    bool mPermaDirty;
    
    //
    // Flag determines if the document has been modified and should be saved to disk
    // The flag is the result of mPermaDirty || !mUndoStack.isClean()
    //
    bool mModified;

};

