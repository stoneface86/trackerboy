
#pragma once

#include "core/Module.hpp"

#include <QString>


//
// File information about a module. Also provides methods for saving/loading
//
class ModuleFile {

public:

    ModuleFile();

    // open the module, true is returned on success. On failure the document
    // is reverted to a new document.
    bool open(QString const& filename, Module &mod);

    //
    // saves the document to the previously loaded/saved file
    //
    bool save(Module &mod);

    //
    // saves the document to the given filename and updates the document's path
    //
    bool save(QString const& filename, Module &mod);

    //
    // Saves a copy of the given module data using this module's file info.
    // The file path of the saved copy is returned on success, amy empty string is
    // returned on failure.
    //
    QString crashSave(Module &mod);

    //
    // Gets the last error that occurred from saving or loading. If no such
    // error occurred, FormatError::none is returned
    //
    trackerboy::FormatError lastError() const;

    //
    // Returns true if the last load/save operation resulted in an I/O error
    //
    bool hasIoError() const;

    //
    // Name of the document, the module filename
    //
    QString name() const noexcept;

    //
    // Set the document's name. The save and open methods will set the name
    // on success.
    //
    void setName(QString const& name) noexcept;

    //
    // Removes the associated filepath
    //
    void clearFilepath() noexcept;

    //
    // Filepath of the module if present. This path is used by save()
    //
    QString filepath() const noexcept;

    //
    // Returns true if the document has a filepath set (ie open or save(filename) was
    // successfully called).
    //
    bool hasFile() const noexcept;

    //
    // Set auto-backup enable. If enabled, existing files will be copied to a .bak file
    // before saving. ie if the user saves "foo.tbm" the current file will be copied to
    // "foo.tbm.bak" in the same directory as foo.tbm. Existing bak files will be overwritten
    //
    void setAutoBackup(bool backup);

private:

    bool doSave(QString const& filename, Module &mod);

    void updateFilename(QString const& path);

    QString mFilename;
    QString mFilepath;

    bool mIoError;
    trackerboy::FormatError mLastError;

    bool mAutoBackup;
};
