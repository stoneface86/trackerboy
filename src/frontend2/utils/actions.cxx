
#include "utils/actions.hxx"

ActionData getData(QAction const *act) {
    auto const data = act->data();
    if (data.canConvert<ActionData>()) {
        return data.value<ActionData>();
    } else {
        return {};
    }
}

ActionBuilder buildAction(QWidget *w, const QString &text, const QString &tip) {
    ActionBuilder result;
    result.action = w->addAction(text);
    result.action->setStatusTip(tip);
    result.data.canShortcut = true;
    return result;
}

ActionBuilder buildAction(QWidget *w, QAction *act) {
    ActionBuilder result;
    result.action = act;
    result.data.canShortcut = true;
    w->addAction(act);
    return result;
}
