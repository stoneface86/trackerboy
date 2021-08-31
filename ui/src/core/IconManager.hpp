
#pragma once

#include <QIcon>
#include <QSize>

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
    trackerRestart,
    trackerPlayRow,
    trackerStepRow,
    trackerStop,
    trackerRecord,
    trackerRepeat,
    ch1,
    ch2,
    ch3,
    ch4,
    increment,
    decrement,
    moveUp,
    moveDown,

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

    static QIcon const& getIcon(Icons icon);

    static constexpr QSize size() {
        return { 16, 16 };
    }

private:

    IconManager();

    static IconManager& getInstance() noexcept;

    std::array<QIcon, static_cast<size_t>(Icons::noIcon)> mIconTable;
    QIcon mPlaceholder;


};
