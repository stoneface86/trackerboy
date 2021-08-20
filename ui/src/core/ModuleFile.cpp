
#include "core/ModuleFile.hpp"

#include <QFileInfo>

#include <fstream>

ModuleFile::ModuleFile() :
    mFilename(),
    mFilepath(),
    mLastError(trackerboy::FormatError::none)
{
}

bool ModuleFile::open(QString const& path, Module &mod) {
    
    std::ifstream in(path.toStdString(), std::ios::binary | std::ios::in);
    mIoError = in.fail();

    if (!mIoError) {

        mLastError = mod.data().deserialize(in);
        mIoError = in.fail();
        if (mLastError == trackerboy::FormatError::none) {
            updateFilename(path);
            // emits the reset signal
            mod.reset();
            return true;
        } else {
            // failed to deserialize module but the module might be paritially loaded
            // clear it
            mod.clear();
        }
    }

    return false;
}

bool ModuleFile::save(Module &mod) {
    if (mFilepath.isEmpty()) {
        return false;
    } else {
        return doSave(mFilepath, mod);
    }
}

bool ModuleFile::save(QString const& filename, Module &mod) {
    auto result = doSave(filename, mod);
    if (result) {
        updateFilename(filename);
    }

    return result;
   
}

trackerboy::FormatError ModuleFile::lastError() const {
    return mLastError;
}

bool ModuleFile::hasIoError() const {
    return mIoError;
}

QString ModuleFile::name() const noexcept {
    return mFilename;
}

void ModuleFile::setName(QString const& name) noexcept {
    if (mFilename != name) {
        mFilename = name;
    }
}

QString ModuleFile::filepath() const noexcept {
    return mFilepath;
}

bool ModuleFile::hasFile() const noexcept {
    return !mFilepath.isEmpty();
}

bool ModuleFile::doSave(QString const& filename, Module &mod) {
    bool success = false;
    std::ofstream out(filename.toStdString(), std::ios::binary | std::ios::out);
    if (out.good()) {
        success = mod.data().serialize(out) == trackerboy::FormatError::none;
        if (success) {
            mod.clean();
        }

    }

    out.close();
    return success;
}

void ModuleFile::updateFilename(QString const& path) {
    mFilepath = path;
    QFileInfo info(path);
    auto filename = info.fileName();
    setName(filename);
}
