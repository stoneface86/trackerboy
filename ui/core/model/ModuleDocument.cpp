
#include "core/model/ModuleDocument.hpp"

#include <QFileInfo>

template <bool tPermanent>
ModuleDocument::EditContext<tPermanent>::EditContext(ModuleDocument &document) :
    mDocument(document)
{
    mDocument.mSpinlock.lock();
}

template <bool tPermanent>
ModuleDocument::EditContext<tPermanent>::~EditContext() {
    mDocument.mSpinlock.unlock();
    if constexpr (tPermanent) {
        mDocument.makeDirty();
    }
}

template class ModuleDocument::EditContext<true>;
template class ModuleDocument::EditContext<false>;



ModuleDocument::ModuleDocument(Spinlock &spinlock, QObject *parent) :
    QObject(parent),
    mPermaDirty(false),
    mModified(false),
    mModule(),
    mSpinlock(spinlock)
{
    clear();
    connect(&mUndoStack, &QUndoStack::cleanChanged, this, &ModuleDocument::onStackCleanChanged);
}

void ModuleDocument::clear() {
    
    mUndoStack.clear();
    mModule.clear();

    clean();

    // always start with 1 song
    auto &songs = mModule.songs();
    songs.emplace_back();
    
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

QUndoStack& ModuleDocument::undoStack() {
    return mUndoStack;
}

void ModuleDocument::abandonStack() {
    if (!mUndoStack.isClean()) {
        makeDirty();
    }
    mUndoStack.clear();
}

ModuleDocument::EditContext<true> ModuleDocument::beginEdit() {
    return { *this };
}

ModuleDocument::EditContext<false> ModuleDocument::beginCommandEdit() {
    return { *this };
}

trackerboy::FormatError ModuleDocument::open(QString const& filename) {
    trackerboy::FormatError error = trackerboy::FormatError::none;
    std::ifstream in(filename.toStdString(), std::ios::binary | std::ios::in);
    if (in.good()) {
        error = mModule.deserialize(in);
        if (error == trackerboy::FormatError::none) {
            clean();
        }
    }

    in.close();
    return error;
    
}

bool ModuleDocument::save(QString const& filename) {
    bool success = false;
    std::ofstream out(filename.toStdString(), std::ios::binary | std::ios::out);
    if (out.good()) {
        success = mModule.serialize(out) == trackerboy::FormatError::none;
        if (success) {
            clean();
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

void ModuleDocument::makeDirty() {
    if (!mPermaDirty) {
        mPermaDirty = true;
        if (!mModified) {
            mModified = true;
            emit modifiedChanged(true);
        }
    }
}

void ModuleDocument::onStackCleanChanged(bool clean) {
    bool modified = mPermaDirty || !clean;
    if (mModified != modified) {
        mModified = modified;
        emit modifiedChanged(mModified);
    }
}

void ModuleDocument::clean() {
    mPermaDirty = false;
    if (mModified) {
        mModified = false;
        emit modifiedChanged(false);
    }
    mUndoStack.setClean();
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



