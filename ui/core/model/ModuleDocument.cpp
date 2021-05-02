
#include "core/model/ModuleDocument.hpp"

#include <QFileInfo>

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
    mWaveModel(*this),
    mLastError(trackerboy::FormatError::none),
    mFilename(),
    mFilepath(),
    mWindow(nullptr)
{
    clear();
    connect(&mUndoStack, &QUndoStack::cleanChanged, this, &ModuleDocument::onStackCleanChanged);
}

ModuleDocument::ModuleDocument(QString const& path, QObject *parent) :
    ModuleDocument(parent)
{
    std::ifstream in(path.toStdString(), std::ios::binary | std::ios::in);
    if (in.good()) {
        mLastError = mModule.deserialize(in);
        if (mLastError == trackerboy::FormatError::none) {

            updateFilename(path);

            mTitle = QString::fromStdString(mModule.title());
            mArtist = QString::fromStdString(mModule.artist());
            mCopyright = QString::fromStdString(mModule.copyright());
            mComments = QString::fromStdString(mModule.comments());

            mInstrumentModel.reload();
            mWaveModel.reload();
            mOrderModel.reload();
        }
    }

    in.close();
}


void ModuleDocument::clear() {
    
    mUndoStack.clear();
    mModule.clear();

    clean();
    
}

trackerboy::FormatError ModuleDocument::lastError() {
    return mLastError;
}

QString ModuleDocument::name() const noexcept {
    return mFilename;
}

void ModuleDocument::setName(QString const& name) noexcept {
    mFilename = name;
}

QString ModuleDocument::filepath() const noexcept {
    return mFilepath;
}

bool ModuleDocument::hasFile() const noexcept {
    return !mFilepath.isEmpty();
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

InstrumentListModel& ModuleDocument::instrumentModel() noexcept {
    return mInstrumentModel;
}

OrderModel& ModuleDocument::orderModel() noexcept {
    return mOrderModel;
}

WaveListModel& ModuleDocument::waveModel() noexcept {
    return mWaveModel;
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

bool ModuleDocument::save() {
    if (mFilepath.isEmpty()) {
        return false;
    } else {
        return doSave(mFilepath);
    }
}

bool ModuleDocument::save(QString const& filename) {
    auto result = doSave(filename);
    if (result) {
        updateFilename(filename);
    }

    return result;
   
}

bool ModuleDocument::doSave(QString const& filename) {
    bool success = false;
    std::ofstream out(filename.toStdString(), std::ios::binary | std::ios::out);
    if (out.good()) {
        mInstrumentModel.commit();
        mWaveModel.commit();

        mModule.setTitle(mTitle.toStdString());
        mModule.setArtist(mArtist.toStdString());
        mModule.setCopyright(mCopyright.toStdString());
        mModule.setComments(mComments.toStdString());

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

void ModuleDocument::updateFilename(QString const& path) {
    mFilepath = path;
    QFileInfo info(path);
    mFilename = info.fileName();
}

void ModuleDocument::setWindow(QMdiSubWindow *window) {
    mWindow = window;
}

QMdiSubWindow* ModuleDocument::window() const noexcept {
    return mWindow;
}

QString ModuleDocument::title() const noexcept {
    return mTitle;
}

QString ModuleDocument::artist() const noexcept {
    return mArtist;
}

QString ModuleDocument::copyright() const noexcept {
    return mCopyright;
}

QString ModuleDocument::comments() const noexcept {
    return mComments;
}

void ModuleDocument::setTitle(QString const& title) {
    mTitle = title;
    makeDirty();
}

void ModuleDocument::setArtist(QString const& artist) {
    mArtist = artist;
    makeDirty();
}

void ModuleDocument::setCopyright(QString const& copyright) {
    mCopyright = copyright;
    makeDirty();
}

void ModuleDocument::setComments(QString const& comments) {
    mComments = comments;
    makeDirty();
}
