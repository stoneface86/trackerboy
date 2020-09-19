
#include "ModuleDocument.hpp"

#include <QFileInfo>

ModuleDocument::ModuleDocument(QObject *parent) :
    mModified(false),
    mModule(),
    mUndoStack(new QUndoStack(this)),
    QObject(parent)
{
}

void ModuleDocument::clear() {
    mModule.clear();

    mUndoStack->clear();

    // always start with 1 song
    auto &songs = mModule.songs();
    songs.emplace_back();
    mCurrentSong = &songs[0];
    mCurrentOrder = 0;
    mCurrentRow = 0;
    
    setModified(false);
}

bool ModuleDocument::isModified() const {
    return mModified;
}

trackerboy::InstrumentTable& ModuleDocument::instrumentTable() {
    return mModule.instrumentTable();
}

trackerboy::WaveTable& ModuleDocument::waveTable() {
    return mModule.waveTable();
}

trackerboy::FormatError ModuleDocument::open(QString filename) {
    trackerboy::FormatError error = trackerboy::FormatError::none;
    std::ifstream in(filename.toStdString(), std::ios::binary | std::ios::in);
    if (in.good()) {
        error = mModule.deserialize(in);
        if (error == trackerboy::FormatError::none) {
            setModified(false);
        }
    }

    in.close();
    return error;
    
}

bool ModuleDocument::save(QString filename) {
    bool success = false;
    std::ofstream out(filename.toStdString(), std::ios::binary | std::ios::out);
    if (out.good()) {
        success = mModule.serialize(out) == trackerboy::FormatError::none;
        if (success) {
            setModified(false);
            mUndoStack->setClean();
        }

    }

    out.close();
    return success;
}


void ModuleDocument::setModified(bool value) {
    if (mModified != value) {
        mModified = value;
        emit modifiedChanged(value);
    }
}





