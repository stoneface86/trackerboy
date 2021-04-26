
#pragma once

// forward declaration (model classes and the document depend on each other)
class ModuleDocument;

#include "core/Spinlock.hpp"
#include "core/model/OrderModel.hpp"
#include "core/model/InstrumentListModel.hpp"
#include "core/model/WaveListModel.hpp"

#include "trackerboy/data/Module.hpp"

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

    explicit ModuleDocument(QObject *parent = nullptr);
    explicit ModuleDocument(QString const& path, QObject *parent = nullptr);


    trackerboy::FormatError lastError();

    QString name() const noexcept;

    void setName(QString const& name) noexcept;

    bool hasFile() const noexcept;

    // accessors for the underlying module data containers

    trackerboy::Module& mod();

    QUndoStack& undoStack();

    // models

    InstrumentListModel& instrumentModel() noexcept;

    OrderModel& orderModel() noexcept;

    WaveListModel& waveModel() noexcept;

    bool isModified() const;

    //
    // Clear the undo stack, if the stack was not clean the perma dirty flag is set
    // Commented out since it is not being used and I don't know why it exists in the first place
    //void abandonStack();

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

    //trackerboy::FormatError open(QString const& filename);

    // saves the document to the previously loaded/saved file
    bool save();

    // saves the document to the given filename and updates the document's path
    bool save(QString const& filename);

    bool tryLock();

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
    Q_DISABLE_COPY(ModuleDocument)

    void makeDirty();
    
    void clean();

    void updateFilename(QString const& path);

    bool doSave(QString const& path);

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

    Spinlock mSpinlock;
    
    QUndoStack mUndoStack;

    // models
    InstrumentListModel mInstrumentModel;
    OrderModel mOrderModel;
    WaveListModel mWaveModel;

    trackerboy::FormatError mLastError;

    QString mFilename;
    QString mFilepath;

};
