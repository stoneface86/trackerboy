
#pragma once

#include <QIcon>
#include <QSize>


//
// enum of icons used throughout the application
//
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
    next,
    prev,

    noIcon,

    // icons that share the same file(s)
    itemExport = fileSave
};

//
// Contains functions for locating icons
//
namespace IconLocator {

//
// Gets an icon from the given Icons enum
//
QIcon get(Icons icon);

//
// Gets the application icon
//
QIcon getAppIcon();

//
// Gets the size of all icons retrieved from this utility
//
constexpr QSize size() {
    return { 16, 16 };
}

}

