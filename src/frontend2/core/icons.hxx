
#pragma once

#include "core/ColorTheme.hxx"
#include <QIcon>
#include <QToolBar>

namespace icons {

//
// Enumeration of all icons available to use in the application
//
enum Icons {
    // Large + Small icons
    New,
    Open,
    Save,
    Config,
    Cut,
    Copy,
    Paste,
    Play,
    PlayStart,
    PlayCursor,
    Step,
    Stop,
    Repeat,
    Record,
    Sidebar,
    Databar,
    // Large + Small + Disabled icons
    Undo,
    Redo,
    SongPrev,
    SongNext,
    Add,
    Remove,
    Duplicate,
    Up,
    Down,
    // Small + Disabled icons
    Import,
    Export,
    Edit,
    // Small icons
    Increment,
    Decrement,
    AllTracks,
    Ch1,
    Ch2,
    Ch3,
    Ch4,
    Count
};

//
// QSize constant for the large sized icons
//
static constexpr QSize largeSize(24, 24);

//
// QSize constant for the small sized icons
//
static constexpr QSize smallSize(16, 16);

//
// Create a new QToolBar with the given parent that will use the large sized
// icons.
//
QToolBar *largeToolBar(QWidget *parent = nullptr);

//
// Create a new QToolBar with the given parent that will use the small sized
// icons.
//
QToolBar *smallToolBar(QWidget *parent = nullptr);

//
// Gets a QIcon containing the icon pixmaps for the given Icons index
//
QIcon get(Icons icon);

//
// Gets a QIcon containing the application icon.
//
QIcon getApp();

///
/// Generates all icons with the provided color theme. You must call this
/// before `get` otherwise null icons will be returned.
///
/// Call this function again and replace existing icons in widgets if the user
/// changes the color theme.
///
void generate(ColorTheme const &theme);

} // namespace icons
