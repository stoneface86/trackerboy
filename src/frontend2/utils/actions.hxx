///
/// Utility functions for creating QActions for a QWidget
///
#pragma once

#include "core/icons.hxx"
#include "utils/aliases.hxx"

#include <QAction>
#include <QString>
#include <QWidget>

///
/// Additional data associated with actions in the application.
///
struct ActionData {
    ActionData(i8 icon = -1, bool canShortcut = false)
        : icon(icon)
        , canShortcut(canShortcut) {}

    i8 icon;          // index of the icon to get or -1 for no icon
    bool canShortcut; // if true, then the user can set a key sequence shortcut
};

//
// Utility struct for initializing a QAction using the builder pattern.
//
struct ActionBuilder {
    QAction *action;
    ActionData data;

    ~ActionBuilder();

    //
    // Sets the icon for the action
    //
    ActionBuilder &icon(icons::Icons icon);

    //
    // Sets the action's shortcut key sequence
    //
    ActionBuilder &shortcut(QKeySequence const &seq);

    //
    // Marks the action has having no user-configurable shortcut
    //
    ActionBuilder &noShortcut();

    //
    // Connects the action's triggered signal to the given slot
    //
    template <typename... Args>
    ActionBuilder &triggers(Args &&...args) {
        QAction::connect(action, &QAction::triggered, args...);
        return *this;
    }

    //
    // Sets the action's parent
    //
    ActionBuilder &parent(QObject *parent);

    //
    // Adds the action to the given widget
    //
    ActionBuilder &addTo(QWidget *widget);
};

///
/// Gets the `ActionData` from the given action data, if set. If there was no
/// data set then default settings are returned.
///
ActionData getData(QAction const *act);

//
// Create an ActionBuilder by adding a new action to the given widget with the
// given text and status tip.
//
ActionBuilder buildAction(QWidget *w, QString const &text, QString const &tip);

//
// Create an ActionBuilder by adding an existing action to the given widget.
// Widget `w` does not take ownership of `act`.
//
ActionBuilder buildAction(QWidget *w, QAction *act);

template <typename... Args>
inline QAction *addAction(QWidget *w, QString const &text, QString const &tip,
                          Args &&...args) {
    auto result = w->addAction(text, args...);
    result->setStatusTip(tip);
    return result;
}

template <typename... Args>
inline QAction *addAction(QWidget *w, QString const &text, QString const &tip,
                          ActionData data, Args &&...args) {
    auto result = addAction(w, text, tip, args...);
    result->setData(data);
    return result;
}
