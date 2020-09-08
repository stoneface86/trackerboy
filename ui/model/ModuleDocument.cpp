
#include "ModuleDocument.hpp"

#include <QFileInfo>

ModuleDocument::ModuleDocument(QObject *parent) :
    mModified(false),
    mModule(),
    mWaveListModel(new WaveListModel(mModule.waveTable(), this)),
    mUndoStack(new QUndoStack(this)),
    QObject(parent)
{
}

void ModuleDocument::clear() {
    mWaveListModel->setEnabled(false);
    mModule.clear();
    mWaveListModel->setEnabled(true);

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

trackerboy::FormatError ModuleDocument::open(QString filename) {
    trackerboy::FormatError error = trackerboy::FormatError::none;
    std::ifstream in(filename.toStdString(), std::ios::binary | std::ios::in);
    if (in.good()) {
        mWaveListModel->setEnabled(false);
        error = mModule.deserialize(in);
        mWaveListModel->setEnabled(true);
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
        //mWaveListModel->setEnabled(false);
        success = mModule.serialize(out) == trackerboy::FormatError::none;
        //mWaveListModel->setEnabled(true);
        if (success) {
            setModified(false);
            mUndoStack->setClean();
        }

    }

    out.close();
    return success;
}


void ModuleDocument::addInstrument() {
    
}

void ModuleDocument::addWaveform() {
    setModified(true);
    mWaveListModel->addItem();
}

void ModuleDocument::setModified(bool value) {
    if (mModified != value) {
        mModified = value;
        emit modifiedChanged(value);
    }
}





