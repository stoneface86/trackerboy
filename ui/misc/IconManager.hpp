
#pragma once

#include <QIcon>

#include <array>

enum class Icons : int {
    fileNew,
    fileOpen,
    fileSave,
    editCut,
    editCopy,
    editPaste,
    itemAdd,
    itemRemove,
    itemDuplicate,
    itemImport,
    itemExport,
    itemEdit,
    trackerPlay,
    trackerPlayPattern,
    trackerStop,
    trackerEdit,
    ch1,
    ch2,
    ch3,
    ch4,
    moveUp,
    moveDown,
    previous,
    next,

    __last = next
};

// singleton class for loading and retreiving QIcons
class IconManager {

public:
    IconManager(IconManager const&) = delete;
    void operator=(IconManager const&) = delete;

    static QIcon& getIcon(Icons icon);

private:

    IconManager();

    static IconManager& getInstance() noexcept;

    std::array<QIcon, static_cast<size_t>(Icons::__last) + 1> mIconTable;
    QIcon mPlaceholder;


};
