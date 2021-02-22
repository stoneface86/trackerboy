
#pragma once

#include "core/Spinlock.hpp"

#include "trackerboy/data/Module.hpp"
#include "trackerboy/engine/RuntimeContext.hpp"

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QUndoStack>

//
// Class encapsulates a trackerboy "document", or a module. Provides methods
// for modifying module data, as well as file I/O. Models for data view widgets
// are also provided.
//
// Regarding thread-safety:
// There are two threads that access the document:
//  * The GUI thread    (read/write)
//  * The render thread (read-only)
// When accessing the document, the document's spinlock should be locked so that
// the render thread does not read while the gui thread (ie the user) modifies
// data. Only the gui thread modifies the document, so locking is not necessary
// when the gui is reading.
//
// Any class that modifies the document's data outside of this class (ie Model classes)
// should lock the document when making changes. Use the beginEdit() to get an edit context
// object that will automatically lock and unlock the spinlock.
//
class ModuleDocument : public QObject {

    Q_OBJECT

public:

    //
    // Utility class when editing the document. On construction, the document's
    // spinlock is locked and then unlocked on destruction. Changes being made 
    // to the document should occur during the scope of an EditContext. This way
    // the locking and unlocking of the spinlock is managed by the context's
    // lifetime.
    //
    template <bool tPermanent = true>
    class EditContext {

    public:
        EditContext(ModuleDocument &document);
        ~EditContext();


    private:
        
        ModuleDocument &mDocument;
    };

    ModuleDocument(QObject *parent = nullptr);

    // accessors for the underlying module data containers

    trackerboy::InstrumentTable& instrumentTable();

    trackerboy::WaveTable& waveTable();

    std::vector<trackerboy::Song> &songs();

    QUndoStack& undoStack();

    bool isModified() const;

    //
    // Clear the undo stack, if the stack was not clean the perma dirty flag is set
    //
    void abandonStack();

    //
    // Utility method constructs an edit context. markModified sets the perma
    // dirty flag if true.
    //
    EditContext<true> beginEdit();

    //
    // Same as beginEdit, but does not set the perma dirty flag. Use this method
    // when redo/undo'ing QUndoCommands
    //
    EditContext<false> beginCommandEdit();

    trackerboy::FormatError open(QString const& filename);

    // saves the document to the current filename
    bool save(QString const& filename);

    void makeDirty();

    void lock();

    void unlock();

signals:
    void modifiedChanged(bool value);

public slots:
    //
    // Clears the document to the default state. Call this slot when creating a new document
    //
    void clear();

private slots:
    void onStackCleanChanged(bool clean);

private:

    void clean();

    // permanent dirty flag. Not all edits to the document can be undone. When such
    // edit occurs, this flag is set to true. It is reset when the document is
    // saved or when the document is reset or loaded from disk.
    bool mPermaDirty;
    
    //
    // Flag determines if the document has been modified and should be saved to disk
    // The flag is the result of mPermaDirty || !mUndoStack.isClean()
    //
    bool mModified;
    trackerboy::Module mModule;

    QMutex mMutex;

    //Spinlock &mSpinlock;
    
    QUndoStack mUndoStack;

};
