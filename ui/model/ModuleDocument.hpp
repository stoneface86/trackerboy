
#pragma once

#include <QObject>
#include <QUndoStack>

#include "trackerboy/data/Module.hpp"
#include "trackerboy/File.hpp"

#include "model/ModuleModel.hpp"


//
// Class encapsulates a trackerboy "document", or a module. Provides methods
// for modifying module data, as well as file I/O. Models for data view widgets
// are also provided.
//
class ModuleDocument : public QObject {

    Q_OBJECT

public:
    ModuleDocument(QObject *parent = nullptr);

    bool isModified() const;

    //void save(QString)

signals:
    void modifiedChanged(bool value);

public slots:
    void clear();


private:

    void setModified(bool value);

    bool mModified;
    
    trackerboy::Module mModule;
    trackerboy::File mFile;

    ModuleModel *mModel;

    // the undo stack is for the current song, the stack is
    // cleared when changing songs
    QUndoStack *mUndoStack;


    // document properties
    trackerboy::Song *mCurrentSong;
    uint8_t mCurrentOrder;
    uint8_t mCurrentRow;
    

};
