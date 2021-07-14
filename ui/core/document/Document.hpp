
#pragma once

//#include "core/model/InstrumentListModel.hpp"
//#include "core/model/OrderModel.hpp"
//#include "core/model/SongModel.hpp"
//#include "core/model/PatternModel.hpp"
//#include "core/model/WaveListModel.hpp"

#include "trackerboy/data/Module.hpp"

#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QString>
#include <QUndoStack>

//
// Class encapsulates a trackerboy "document", or a module. Provides methods
// for modifying module data, as well as file I/O. Models for data view widgets
// are also provided.
//
// Regarding thread-safety:
// There are two threads that access the document:
//  * The GUI thread    (read/write)
//  * The render thread (read-only)
// When accessing the document, the document's mutex should be locked so that
// the render thread does not read while the gui thread (ie the user) modifies
// data. Only the gui thread modifies the document, so locking is not necessary
// when the gui is reading. The locker() should be used to get a QMutexLocker
// object when accessing the module in other threads.
//
// Any class that modifies the document's data outside of this class (ie Model classes)
// should lock the document when making changes. The DocumentEditor class is to be used
// when making changes.
//
class Document : public QObject {

    Q_OBJECT

public:

    //
    // These flags determine which channels are enabled for music playback
    // Typically used for solo'ing a channel by disabling all channels except
    // for one.
    //
    enum OutputFlag {
        AllOff = 0x0,
        CH1 = 0x1,
        CH2 = 0x2,
        CH3 = 0x4,
        CH4 = 0x8,
        AllOn = CH1 | CH2 | CH3 | CH4
    };
    Q_DECLARE_FLAGS(OutputFlags, OutputFlag)

    explicit Document(QObject *parent = nullptr);
    virtual ~Document() = default;

    // file management -------------------------------------------------------

    // reset the document to a new one
    void clear();

    // open the module, true is returned on success. On failure the document
    // is reverted to a new document.
    bool open(QString const& filename);

    //
    // saves the document to the previously loaded/saved file
    //
    bool save();

    //
    // saves the document to the given filename and updates the document's path
    //
    bool save(QString const& filename);

    //
    // Gets the last error that occurred from saving or loading
    //
    trackerboy::FormatError lastError();

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
    // Filepath of the module if present. This path is used by save()
    //
    QString filepath() const noexcept;

    //
    // Returns true if the document has a filepath set (ie open or save(filename) was
    // successfully called).
    //
    bool hasFile() const noexcept;

    // Properties ------------------------------------------------------------

    trackerboy::Module const& mod() const;

    QUndoStack& undoStack();

    //
    // Returns true if the document is modified.
    //
    bool isModified() const;

    //
    // Non-GUI threads must acquire a locker before accessing the
    // document's module
    //
    QMutexLocker locker();

    // Models ----------------------------------------------------------------

    //InstrumentListModel& instrumentModel() noexcept;

    //OrderModel& orderModel() noexcept;

    //ModuleModel& moduleModel() noexcept; // module settings

    //PatternModel& patternModel() noexcept;

    //SongModel& songModel() noexcept;

    //WaveListModel& waveModel() noexcept;

    // Application settings --------------------------------------------------

    // these settings are set by the user, and are not saved in the module

    OutputFlags channelOutput() const;

    int editStep() const;
    
    int instrument() const;
    
    bool keyRepetition() const;
    
    void setChannelOutput(OutputFlags flags);
    
    void setEditStep(int editStep);

    void setInstrument(int instrument);
    
    void setKeyRepetition(bool enable);

    void toggleChannelOutput();

    void solo();
    
signals:

    void modifiedChanged(bool value);

    void channelOutputChanged(OutputFlags flags);

    void reloaded();

    void nameChanged(QString const& name);

private:
    friend class DocumentEditor;

    Q_DISABLE_COPY(Document)

    void clean();

    bool doSave(QString const& path);

    void makeDirty();

    void updateFilename(QString const& path);

// ~~~ CLASS MEMBERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    QMutex mMutex;

    QUndoStack mUndoStack;
    trackerboy::Module mModule;

    // permanent dirty flag. Not all edits to the document can be undone. When such
    // edit occurs, this flag is set to true. It is reset when the document is
    // saved or when the document is reset or loaded from disk.
    bool mPermaDirty;
    
    //
    // Flag determines if the document has been modified and should be saved to disk
    // The flag is the result of mPermaDirty || !mUndoStack.isClean()
    //
    bool mModified;

    // models
    //InstrumentListModel mInstrumentModel;
    //OrderModel mOrderModel;
    //SongModel mSongModel;
    //PatternModel mPatternModel;
    //WaveListModel mWaveModel;

    trackerboy::FormatError mLastError;

    QString mFilename;
    QString mFilepath;

     // bitmap for channels that are outputting sound.
    OutputFlags mChannelEnables;

    bool mKeyRepetition;
    int mEditStep;
    int mInstrument;

};