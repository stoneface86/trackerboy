
#include "ModuleDocument.hpp"

ModuleDocument::ModuleDocument(QObject *parent) :
    mModified(true),
    mModule(),
    mWaveListModel(new WaveListModel(mModule.waveTable(), this)),
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

WaveListModel* ModuleDocument::waveListModel() {
    return mWaveListModel;
}


void ModuleDocument::addInstrument() {
    
}

void ModuleDocument::addWaveform() {
    mWaveListModel->addItem();
}

void ModuleDocument::setModified(bool value) {
    if (mModified != value) {
        mModified = value;
        emit modifiedChanged(value);
    }
}





