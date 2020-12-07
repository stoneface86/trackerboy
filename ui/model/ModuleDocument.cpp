
#include "ModuleDocument.hpp"

#include <QFileInfo>

ModuleDocument::EditContext::EditContext(ModuleDocument &document, bool markModified) :
    mDocument(document),
    mMarkModified(markModified)
{
    mDocument.lock();
}

ModuleDocument::EditContext::~EditContext() {
    mDocument.unlock();
    if (mMarkModified) {
        mDocument.setModified(true);
    }
}



ModuleDocument::ModuleDocument(QObject *parent) :
    QObject(parent),
    mModified(false),
    mModule(),
    mUndoStack(new QUndoStack(this)),
    mSpinlock()
{
    clear();
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

std::vector<trackerboy::Song>& ModuleDocument::songs() {
    return mModule.songs();
}

ModuleDocument::EditContext ModuleDocument::beginEdit(bool markModified) {
    return { *this, markModified };
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

void ModuleDocument::lock() {
    mSpinlock.lock();
}

bool ModuleDocument::trylock() {
    return mSpinlock.tryLock();
}

void ModuleDocument::unlock() {
    mSpinlock.unlock();
}



