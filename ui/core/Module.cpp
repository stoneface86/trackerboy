
#include "core/Module.hpp"


Module::Editor::Editor(Module &mod) :
    QMutexLocker(&mod.mMutex)
{
}

Module::PermanentEditor::PermanentEditor(Module &mod) :
    Editor(mod),
    mModule(mod)
{
}

Module::PermanentEditor::~PermanentEditor() {
    unlock();
    mModule.makeDirty();
}

Module::Module(QObject *parent) :
    QObject(parent),
    mModule(),
    mMutex(),
    mUndoStack(),
    mPermaDirty(false),
    mModified(false)
{
    connect(&mUndoStack, &QUndoStack::cleanChanged, this,
        [this](bool clean) {
            bool modified = mPermaDirty || !clean;
            if (mModified != modified) {
                mModified = modified;
                emit modifiedChanged(mModified);
            }
        });
}

void Module::clear() {
    mModule.clear();
    reset();
}

trackerboy::Module const& Module::data() const {
    return mModule;
}

trackerboy::Module& Module::data() {
    return mModule;
}

bool Module::isModified() const {
    return mModified;
}

QMutex& Module::mutex() {
    return mMutex;
}

QUndoStack& Module::undoStack() {
    return mUndoStack;
}

void Module::reset() {
    mUndoStack.clear();
    clean();
    emit reloaded();
}

Module::Editor Module::edit() {
    return { *this };
}

Module::PermanentEditor Module::permanentEdit() {
    return { *this };
}

void Module::clean() {
    mPermaDirty = false;
    if (mModified) {
        mModified = false;
        emit modifiedChanged(false);
    }
    mUndoStack.setClean();
}

void Module::makeDirty() {
    if (!mPermaDirty) {
        mPermaDirty = true;
        if (!mModified) {
            mModified = true;
            emit modifiedChanged(true);
        }
    }
}

