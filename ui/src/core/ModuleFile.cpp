
#include "core/ModuleFile.hpp"

#include <QDateTime>
#include <QDir>
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

QString ModuleFile::crashSave(Module &mod) {
    // attempt to save a copy of the module
    // the copy is the same path of the module, but with .crash-%1 appended
    // where %1 is an ISO 8601 timestamp
    QString copyPath;
    if (hasFile()) {
        QFileInfo info(mFilepath);
        copyPath = info.dir().filePath(info.baseName());
    } else {
        copyPath = mFilename;
    }
    copyPath.append(QStringLiteral(".crash-%1.tbm").arg(
        QDateTime::currentDateTime().toString(Qt::ISODate)
    ));

    // This approach uses the same directory as the module file, which will
    // fail if we don't have write permissions in this directory. Might be worth
    // checking beforehand and use a fall-back location if this is the case
    // (perhaps the user's home directory)

    // for modules without a path, the file gets saved in the current directory

    // in the case of collisions, the file will be overwritten. Should be
    // extremely unlikely due to the timestamp being added to the filename

    std::ofstream stream(copyPath.toStdString(), std::ios::binary | std::ios::out);
    if (stream.good()) {
        mod.beginSave();
        if (mod.data().serialize(stream) == trackerboy::FormatError::none) {
            // success! return the path of the saved file
            return copyPath;
        }
    }

    // we either could not open the stream or an error occurred during serialization
    return {};
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
        mod.beginSave();
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
