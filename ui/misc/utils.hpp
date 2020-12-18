
// misc utility functions

#pragma once

#include "misc/IconManager.hpp"

#include <QAction>
#include <QKeySequence>
#include <QString>

//
// Sets the text, statusTip and shortcut for the given action. Equivalent to
// action.setText(text);
// action.setStatusTip(statusTip);
// action.setShortcut(seq);
//
void setupAction(
    QAction &action,
    const char *text,
    const char *statusTip,
    QKeySequence const &seq = QKeySequence()
);

// same as above but set the icon too
void setupAction(
    QAction &action,
    const char *text,
    const char *statusTip,
    Icons icon,
    QKeySequence const &seq = QKeySequence()
);
