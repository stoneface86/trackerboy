
#pragma once

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
    ModuleDocument(QObject *parent = nullptr);

    trackerboy::InstrumentTable& instrumentTable();

    trackerboy::WaveTable& waveTable();

    std::vector<trackerboy::Song> &songs();

    bool isModified() const;

    trackerboy::FormatError open(QString filename);

    // saves the document to the current filename
    bool save(QString filename);

    void setModified(bool value);

    void lock();
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

    QMutex mMutex;


    // document properties
    trackerboy::Song *mCurrentSong;
    uint8_t mCurrentOrder;
    uint8_t mCurrentRow;
    

};
