
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
    mUndoGroup(new QUndoGroup(this)),
    mSongUndoStacks(),
    mSong(),
    mPermaDirty(false),
    mModified(false)
{
    reset();

    connect(mUndoGroup, &QUndoGroup::cleanChanged, this,
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

trackerboy::Song* Module::song() {
    return mSong.get();
}

trackerboy::Song const* Module::song() const {
    return mSong.get();
}

std::shared_ptr<trackerboy::Song> Module::songShared() {
    return mSong;
}

bool Module::isModified() const {
    return mModified;
}

QMutex& Module::mutex() {
    return mMutex;
}

QUndoGroup* Module::undoGroup() {
    return mUndoGroup;
}

QUndoStack* Module::undoStack() {
    return mUndoGroup->activeStack();
}

void Module::reset() {
    resizeUndoStacks(mModule.songs().size());
    setSong(0);
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
    for (auto stack : mSongUndoStacks) {
        stack->setClean();
    }
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

void Module::addSong() {
    mSongUndoStacks.append(new QUndoStack(mUndoGroup));
}

void Module::removeSong(int index) {
    delete mSongUndoStacks.takeAt(index);
}

void Module::setSong(int index) {
    mSong = mModule.songs().getShared(index);
    mUndoGroup->setActiveStack(mSongUndoStacks.at(index));
    emit songChanged();
}

void Module::beginSave() {
    emit aboutToSave();
}

void Module::resizeUndoStacks(int count) {
    int oldcount = mSongUndoStacks.size();
    int endIndexToClear = oldcount;
    
    if (count < oldcount) {
        // shrink, delete and remove the undo stacks
        for (int i = count; i < oldcount; ++i) {
            delete mSongUndoStacks[i];
        }
        mSongUndoStacks.resize(count);
        endIndexToClear = count;
    } else if (count > oldcount) {
        // enlarge, alloc and add the new stacks
        for (int i = count - oldcount; i > 0; --i) {
            addSong();
        }
    }

    // clear the already allocated stacks
    for (int i = 0; i < endIndexToClear; ++i) {
        mSongUndoStacks[i]->clear();
    }
    
}
