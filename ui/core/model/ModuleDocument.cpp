
#include "core/model/ModuleDocument.hpp"

#include <QFileInfo>
#include <QThread>

#include <fstream>

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



ModuleDocument::ModuleDocument(QObject *parent) :
    QObject(parent),
    mPermaDirty(false),
    mModified(false),
    mModule(),
    mSpinlock(),
    mUndoStack(),
    mInstrumentModel(*this),
    mOrderModel(*this),
    mWaveModel(*this)
{
    clear();
    connect(&mUndoStack, &QUndoStack::cleanChanged, this, &ModuleDocument::onStackCleanChanged);
}

void ModuleDocument::clear() {
    
    mUndoStack.clear();
    mModule.clear();

    clean();
    
}

bool ModuleDocument::isModified() const {
    return mModified;
}

trackerboy::Module& ModuleDocument::mod() {
    return mModule;
}

QUndoStack& ModuleDocument::undoStack() {
    return mUndoStack;
}

//void ModuleDocument::abandonStack() {
//    if (!mUndoStack.isClean()) {
//        makeDirty();
//    }
//    mUndoStack.clear();
//}

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

bool ModuleDocument::tryLock() {
    return mSpinlock.tryLock();
}

void ModuleDocument::lock() {
    mSpinlock.lock();
}

void ModuleDocument::unlock() {
    mSpinlock.unlock();
}



