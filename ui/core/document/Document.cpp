
#include "core/document/Document.hpp"


#include <QFileInfo>
#include <fstream>

Document::Document(QObject *parent) :
    QObject(parent),
    mMutex(),
    mUndoStack(),
    mModule(),
    mPermaDirty(false),
    mModified(false),
    //mInstrumentModel(*this),
    //mOrderModel(*this),
    //mSongModel(*this),
    //mPatternModel(*this),
    //mWaveModel(*this),
    mLastError(trackerboy::FormatError::none),
    mFilename(),
    mFilepath(),
    mChannelEnables(CH1 | CH2 | CH3 | CH4),
    mKeyRepetition(true),
    mEditStep(1),
    mInstrument(0)
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

void Document::clear() {
    mUndoStack.clear();
    mModule.clear();

    mLastError = trackerboy::FormatError::none;
    mFilename.clear();
    mFilepath.clear();
    // mTitle.clear();
    // mArtist.clear();
    // mCopyright.clear();
    // mComments.clear();
    mInstrument = 0;
    setChannelOutput(AllOn);

    clean();

    // mInstrumentModel.reload();
    // mWaveModel.reload();
    // mOrderModel.reload();
    // mPatternModel.reload();
    // mSongModel.reload();

    emit reloaded();
}

bool Document::open(QString const& path) {
    bool success = false;
    std::ifstream in(path.toStdString(), std::ios::binary | std::ios::in);
    if (in.good()) {
        mLastError = mModule.deserialize(in);
        if (mLastError == trackerboy::FormatError::none) {

            updateFilename(path);

            //mTitle = QString::fromStdString(mModule.title());
            //mArtist = QString::fromStdString(mModule.artist());
            //mCopyright = QString::fromStdString(mModule.copyright());
            //mComments = QString::fromStdString(mModule.comments());

            //mInstrumentModel.reload();
            //mWaveModel.reload();
            //mOrderModel.reload();
            //mPatternModel.reload();
            //mSongModel.reload();

            emit reloaded();
            
            success = true;
        } else {
            clear();
        }
    }

    in.close();
    return success;
}

bool Document::save() {
    if (mFilepath.isEmpty()) {
        return false;
    } else {
        return doSave(mFilepath);
    }
}

bool Document::save(QString const& filename) {
    auto result = doSave(filename);
    if (result) {
        updateFilename(filename);
    }

    return result;
   
}

trackerboy::FormatError Document::lastError() {
    return mLastError;
}

QString Document::name() const noexcept {
    return mFilename;
}

void Document::setName(QString const& name) noexcept {
    if (mFilename != name) {
        mFilename = name;
        emit nameChanged(name);
    }
}

QString Document::filepath() const noexcept {
    return mFilepath;
}

bool Document::hasFile() const noexcept {
    return !mFilepath.isEmpty();
}

trackerboy::Module const& Document::mod() const {
    return mModule;
}

QUndoStack& Document::undoStack() {
    return mUndoStack;
}

bool Document::isModified() const {
    return mModified;
}

QMutexLocker Document::locker() {
    return QMutexLocker(&mMutex);
}

// InstrumentListModel& Document::instrumentModel() noexcept {
//     return mInstrumentModel;
// }

// OrderModel& Document::orderModel() noexcept {
//     return mOrderModel;
// }

// PatternModel& Document::patternModel() noexcept {
//     return mPatternModel;
// }

// SongModel& Document::songModel() noexcept {
//     return mSongModel;
// }

// WaveListModel& Document::waveModel() noexcept {
//     return mWaveModel;
// }

Document::OutputFlags Document::channelOutput() const {
    return mChannelEnables;
}

int Document::editStep() const {
    return mEditStep;
}

int Document::instrument() const {
    return mInstrument;
}

bool Document::keyRepetition() const {
    return mKeyRepetition;
}

void Document::setChannelOutput(OutputFlags flags) {
    if (flags != mChannelEnables) {
        mChannelEnables = flags;
        emit channelOutputChanged(flags);
    }
}

void Document::setEditStep(int editStep) {
    if (editStep >= 0) {
        mEditStep = editStep;
    }
}

void Document::setInstrument(int instrument) {
    mInstrument = instrument;
}

void Document::setKeyRepetition(bool repetition) {
    mKeyRepetition = repetition;
}

void Document::toggleChannelOutput() {
    // auto track = mPatternModel.cursorTrack();
    // mChannelEnables ^= (OutputFlag)(1 << track);
    // emit channelOutputChanged(mChannelEnables);
}

void Document::solo() {
    // auto track = mPatternModel.cursorTrack();
    // auto soloFlags = (OutputFlag)(1 << track);
    // if (mChannelEnables == soloFlags) {
    //     mChannelEnables = AllOn;
    // } else {
    //     mChannelEnables = soloFlags;
    // }
    // emit channelOutputChanged(mChannelEnables);
}

void Document::clean() {
    mPermaDirty = false;
    if (mModified) {
        mModified = false;
        emit modifiedChanged(false);
    }
    mUndoStack.setClean();
}

bool Document::doSave(QString const& filename) {
    bool success = false;
    std::ofstream out(filename.toStdString(), std::ios::binary | std::ios::out);
    if (out.good()) {
        //mInstrumentModel.commit();
        //mWaveModel.commit();

        //mModule.setTitle(mTitle.toStdString());
        //mModule.setArtist(mArtist.toStdString());
        //mModule.setCopyright(mCopyright.toStdString());
        //mModule.setComments(mComments.toStdString());

        success = mModule.serialize(out) == trackerboy::FormatError::none;
        if (success) {
            clean();
        }

    }

    out.close();
    return success;
}

void Document::makeDirty() {
    if (!mPermaDirty) {
        mPermaDirty = true;
        if (!mModified) {
            mModified = true;
            emit modifiedChanged(true);
        }
    }
}

void Document::updateFilename(QString const& path) {
    mFilepath = path;
    QFileInfo info(path);
    auto filename = info.fileName();
    setName(filename);
}



// move these to a separate model class

// QString Document::title() const noexcept {
//     return mTitle;
// }

// QString Document::artist() const noexcept {
//     return mArtist;
// }

// QString Document::copyright() const noexcept {
//     return mCopyright;
// }

// QString Document::comments() const noexcept {
//     return mComments;
// }

// void Document::setTitle(QString const& title) {
//     mTitle = title;
//     makeDirty();
// }

// void Document::setArtist(QString const& artist) {
//     mArtist = artist;
//     makeDirty();
// }

// void Document::setCopyright(QString const& copyright) {
//     mCopyright = copyright;
//     makeDirty();
// }

// void Document::setComments(QString const& comments) {
//     mComments = comments;
//     makeDirty();
// }



// float Document::framerate() const {
//     return mModule.framerate();
// }

// void Document::setFramerate(int rate) {
//     if (mModule.system() == trackerboy::System::custom && mModule.customFramerate() == rate) {
//         return;
//     }
    
//     auto ctx = beginEdit();
//     mModule.setFramerate(rate);
//     emit framerateChanged((float)rate);
    
// }

// void Document::setFramerate(trackerboy::System system) {
//     if (system == trackerboy::System::custom || mModule.system() == system) {
//         return;
//     }

//     auto ctx = beginEdit();
//     mModule.setFramerate(system);
//     emit framerateChanged(mModule.framerate());
// }
