
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

ActionBuilder::~ActionBuilder() {
    action->setData(QVariant::fromValue(data));
}

ActionBuilder &ActionBuilder::icon(icons::Icons icon) {
    data.icon = icon;
    return *this;
}

ActionBuilder &ActionBuilder::shortcut(QKeySequence const &seq) {
    action->setShortcut(seq);
    return *this;
}

ActionBuilder &ActionBuilder::noShortcut() {
    data.canShortcut = false;
    return *this;
}

ActionBuilder &ActionBuilder::parent(QObject *parent) {
    action->setParent(parent);
    return *this;
}

ActionBuilder &ActionBuilder::addTo(QWidget *widget) {
    widget->addAction(action);
    return *this;
}
