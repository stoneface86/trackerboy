
#include "core/Module.hpp"


Module::Editor::Editor(Module &mod) :
    QMutexLocker<QMutex>(&mod.mMutex)
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
    mUndoStacks(),
    mSong(),
    mPermaDirty(false),
    mModified(false)
{
    nameFirstSong();
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
    // clear song history
    mUndoStacks.clear();

    mModule.clear();
    nameFirstSong();
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
    auto const stacks = mUndoGroup->stacks();
    for (auto stack : stacks) {
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

void Module::setSong(int index) {
    mSong = mModule.songs().getShared(index);

    QUndoStack *stack;
    auto iter = mUndoStacks.find(mSong.get());
    if (iter != mUndoStacks.end()) {
        // contain the existing stack
        stack = iter->second.get();
    } else {
        // no history for this song yet, create it and add to group
        stack = new QUndoStack(this);
        mUndoGroup->addStack(stack);
        mUndoStacks.emplace(mSong.get(), stack);
    }
    mUndoGroup->setActiveStack(stack);
    emit songChanged();
}

void Module::removeHistory(trackerboy::Song *song) {
    mUndoStacks.erase(song);
}

void Module::beginSave() {
    emit aboutToSave();
}

QString Module::defaultSongName() const {
    return tr("New song");
}

void Module::nameFirstSong() {
    // excuse the jank
    mModule.songs().get(0)->setName(defaultSongName().toStdString());
}
