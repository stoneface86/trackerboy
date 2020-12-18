
#include "misc/utils.hpp"

#include <QCoreApplication>

void setupAction(
    QAction &action,
    const char *text,
    const char *statusTip,
    QKeySequence const &seq
) {
    action.setText(QCoreApplication::tr(text));
    action.setShortcut(seq);
    action.setStatusTip(QCoreApplication::tr(statusTip));
}

void setupAction(
    QAction &action,
    const char *text,
    const char *statusTip,
    Icons icon,
    QKeySequence const &seq
) {

    setupAction(action, text, statusTip, seq);
    action.setIcon(IconManager::getIcon(icon));

}
