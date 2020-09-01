
#include "ModuleDocument.hpp"

ModuleDocument::ModuleDocument(QObject *parent) :
    mModified(true),
    mModule(),
    mFile(),
    mModel(new ModuleModel(mModule, this)),
    mUndoStack(new QUndoStack(this)),
    QObject(parent)
{
}

void ModuleDocument::clear() {
    mModule.clear();
    mUndoStack->clear();

    // always start with 1 song
    auto &st = mModule.songTable();
    mCurrentSong = &st.insert();
    mCurrentOrder = 0;
    mCurrentRow = 0;
    
    setModified(false);
}

bool ModuleDocument::isModified() const {
    return mModified;
}


void ModuleDocument::setModified(bool value) {
    if (mModified != value) {
        mModified = value;
        emit modifiedChanged(value);
    }
}





