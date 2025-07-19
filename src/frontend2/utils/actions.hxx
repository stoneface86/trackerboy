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
        , canShortcut(canShortcut)
        , defaultShortcut() {}

    i8 icon;          // index of the icon to get or -1 for no icon
    bool canShortcut; // if true, then the user can set a key sequence shortcut
    QKeySequence defaultShortcut;
};

//
// Utility struct for initializing a QAction using the builder pattern.
//
struct ActionBuilder {
    QAction *action;
    ActionData data;

    //
    // Sets the action data to the action's data property.
    //
    inline ~ActionBuilder() { action->setData(QVariant::fromValue(data)); }

    //
    // Sets the icon for the action
    //
    inline ActionBuilder &icon(icons::Icons icon) {
        data.icon = icon;
        return *this;
    }

    //
    // Sets the action's shortcut key sequence
    //
    inline ActionBuilder &shortcut(QKeySequence const &seq) {
        action->setShortcut(seq);
        data.defaultShortcut = seq;
        return *this;
    }

    //
    // Marks the action has having no user-configurable shortcut
    //
    inline ActionBuilder &noShortcut() {
        data.canShortcut = false;
        return *this;
    }

    //
    // Connects the action's triggered signal to the given slot
    //
    template <typename... Args>
    inline ActionBuilder &triggers(Args &&...args) {
        QAction::connect(action, &QAction::triggered, args...);
        return *this;
    }

    //
    // Connects the action's toggled signal to the given slot
    //
    template <typename... Args>
    inline ActionBuilder &toggles(Args &&...args) {
        QAction::connect(action, &QAction::toggled, args...);
        return *this;
    }

    //
    // Sets the action's parent
    //
    inline ActionBuilder &parent(QObject *parent) {
        action->setParent(parent);
        return *this;
    }

    //
    // Adds the action to the given widget
    //
    inline ActionBuilder &addTo(QWidget *widget) {
        widget->addAction(action);
        return *this;
    }

    //
    // Assigns the action to the given reference `act`
    //
    inline ActionBuilder &store(QAction *&act) {
        act = action;
        return *this;
    }

    //
    // Sets the action's checkable property to `state`
    //
    inline ActionBuilder &checkable(bool state = true) {
        action->setCheckable(state);
        return *this;
    }

    //
    // Sets the action's checked property to `state`
    //
    inline ActionBuilder &checked(bool state = true) {
        action->setChecked(state);
        return *this;
    }
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

//
// Iterates all actions in a widget, recursively. Each QAction that is not a
// separator or menu will be passed as an argument to the functor `f`.
//
template <typename Func>
void iterateActions(QWidget *widget, Func f) {
    for (const auto action : widget->actions()) {
        if (!action->isSeparator()) {
            auto menu = action->menu();
            if (menu == nullptr) {
                f(action);
            } else {
                iterateActions(menu, f);
            }
        }
    }
}
