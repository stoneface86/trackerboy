
#pragma once

#include "Spinlock.hpp"

#include <QObject>
#include <QMutex>
#include <QUndoStack>
#include <QStringListModel>

#include "trackerboy/data/Module.hpp"
#include "trackerboy/engine/RuntimeContext.hpp"


//
// Class encapsulates a trackerboy "document", or a module. Provides methods
// for modifying module data, as well as file I/O. Models for data view widgets
// are also provided.
//
// Regarding thread-safety:
// There are two threads that access the document:
//  * The GUI thread    (read/write)
//  * The render thread (read-only)
// When accessing the document, the document's mutex should be locked so that
// the render thread does not read while the gui thread (ie the user) modifies
// data. Only the gui thread modifies the document, so locking is not necessary
// when the gui is reading.
//
// Any class that modifies the document's data outside of this class (ie Model classes)
// should lock the document when making changes.
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
    class EditContext {

    public:
        EditContext(ModuleDocument &document, bool markModified);
        ~EditContext();


    private:
        
        ModuleDocument &mDocument;
        bool mMarkModified;
    };

    ModuleDocument(QObject *parent = nullptr);

    trackerboy::InstrumentTable& instrumentTable();

    trackerboy::WaveTable& waveTable();

    std::vector<trackerboy::Song> &songs();

    bool isModified() const;

    EditContext beginEdit(bool markModified = true);

    trackerboy::FormatError open(QString filename);

    // saves the document to the current filename
    bool save(QString filename);

    void setModified(bool value);

    void lock();
    bool trylock();
    void unlock();

signals:
    void modifiedChanged(bool value);

public slots:
    void clear();


private:

    bool mModified;
    
    trackerboy::Module mModule;

    //ModuleModel *mModel;

    // the undo stack is for the current song, the stack is
    // cleared when changing songs
    QUndoStack *mUndoStack;

    //QMutex mMutex;
    Spinlock mSpinlock;


    // document properties
    trackerboy::Song *mCurrentSong;
    uint8_t mCurrentOrder;
    uint8_t mCurrentRow;
    

};
