
#pragma once

#include <QIcon>

#include <array>


enum class Icons : int {
    fileNew,
    fileOpen,
    fileSave,
    fileConfig,
    editUndo,
    editRedo,
    editCut,
    editCopy,
    editPaste,
    itemAdd,
    itemRemove,
    itemDuplicate,
    itemImport,
    itemEdit,
    trackerPlay,
    trackerPause,
    trackerRestart,
    trackerStop,
    patternPlay,
    patternRestart,
    patternRecord,
    patternPlayRow,
    ch1,
    ch2,
    ch3,
    ch4,
    increment,
    decrement,
    moveUp,
    moveDown,
    previous,
    next,

    noIcon,

    // icons that share the same file(s)
    itemExport = fileSave
};

// singleton class for loading and retreiving QIcons
class IconManager {

public:
    IconManager(IconManager const&) = delete;
    void operator=(IconManager const&) = delete;

    static QIcon getAppIcon();

    static QIcon& getIcon(Icons icon);

private:

    IconManager();

    static IconManager& getInstance() noexcept;

    std::array<QIcon, static_cast<size_t>(Icons::noIcon)> mIconTable;
    QIcon mPlaceholder;


};
