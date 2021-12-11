
#pragma once

#include "utils/IconLocator.hpp"

class QAction;
class QMenu;
class QWidget;
#include <QString>
#include <QKeySequence>


//
// adds a new action to a menu and sets the text, status tip and shortcut
//
QAction* setupAction(QMenu *menu, QString const& text, QString const& statusTip, QKeySequence const& seq = QKeySequence());

// same as above, but also sets the icon
QAction* setupAction(QMenu *menu, QString const& text, QString const& statusTip, Icons icon, QKeySequence const &seq = QKeySequence());

QAction* createAction(QWidget *parent, QString const& text, QString const& statusTip, QKeySequence const& seq = QKeySequence());

QAction* createAction(QWidget *parent, QString const& text, QString const& statusTip, Icons icon, QKeySequence const& seq = QKeySequence());
