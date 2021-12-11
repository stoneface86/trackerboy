
#include "utils/actions.hpp"

#include <QMenu>

//
// adds a new action to a menu and sets the text, status tip and shortcut
//
QAction* setupAction(QMenu *menu, QString const& text, QString const& statusTip, QKeySequence const& seq) {
    auto act = menu->addAction(text);
    act->setStatusTip(statusTip);
    act->setShortcut(seq);
    return act;
}

// same as above, but also sets the icon
QAction* setupAction(QMenu *menu, QString const& text, QString const& statusTip, Icons icon, QKeySequence const& seq) {
    auto act = setupAction(menu, text, statusTip, seq);
    act->setIcon(IconLocator::get(icon));
    return act;
}

QAction* createAction(QWidget *parent, QString const& text, QString const& statusTip, QKeySequence const& seq) {
    auto act = new QAction(text, parent);
    act->setStatusTip(statusTip);
    act->setShortcut(seq);
    return act;
}

QAction* createAction(QWidget *parent, QString const& text, QString const& statusTip, Icons icon, QKeySequence const& seq) {
    auto act = createAction(parent, text, statusTip, seq);
    act->setIcon(IconLocator::get(icon));
    return act;
}
