
#pragma once

#include <QObject>
#include <QUndoStack>
#include <QStringListModel>

#include "trackerboy/data/Module.hpp"
#include "trackerboy/engine/RuntimeContext.hpp"

#include "model/InstrumentListModel.hpp"
#include "model/WaveListModel.hpp"


//
// Class encapsulates a trackerboy "document", or a module. Provides methods
// for modifying module data, as well as file I/O. Models for data view widgets
// are also provided.
//
class ModuleDocument : public QObject {

    Q_OBJECT

public:
    ModuleDocument(QObject *parent = nullptr);

    InstrumentListModel* instrumentListModel();
    WaveListModel* waveListModel();

    trackerboy::InstrumentTable& instrumentTable();

    trackerboy::WaveTable& waveTable();

    bool isModified() const;

    trackerboy::FormatError open(QString filename);

    // saves the document to the current filename
    bool save(QString filename);

signals:
    void modifiedChanged(bool value);

public slots:
    void clear();
    void addInstrument();
    void addWaveform();


private:

    void setModified(bool value);


    

    bool mModified;
    
    trackerboy::Module mModule;

    InstrumentListModel *mInstrumentListModel;
    WaveListModel *mWaveListModel;
    
    //ModuleModel *mModel;

    // the undo stack is for the current song, the stack is
    // cleared when changing songs
    QUndoStack *mUndoStack;


    // document properties
    trackerboy::Song *mCurrentSong;
    uint8_t mCurrentOrder;
    uint8_t mCurrentRow;
    

};
