
#include "utils/RecentFiles.hxx"

#include <QFileInfo>

void RecentFiles::setup(QMenu *menu) {
    separator = menu->addSeparator();
    separator->setVisible(false);
    for (auto &recent : actions) {
        recent = new QAction;
        recent->setVisible(false);
        menu->addAction(recent);
    }
}

QStringList RecentFiles::toList() {
    QStringList list;
    for (auto const act : actions) {
        if (act->isVisible()) {
            list.append(act->statusTip());
        } else {
            break;
        }
    }
    return list;
}

QStringList RecentFiles::push(QString const &file) {
    auto list = toList();
    list.removeAll(file);
    list.prepend(file);
    while (list.size() > (int)actions.size()) {
        list.removeLast();
    }
    setFromList(list);
    return list;
}

void RecentFiles::setFromList(QStringList const &list) {
    auto const size = list.size();
    separator->setVisible(size > 0);

    int i = 0;
    for (auto const &filename : list) {
        QFileInfo info(filename);
        QString text(3, ' ');
        text[0] = '&';
        text[1] = QChar('1' + i);
        text.append(info.fileName());
        auto const act = actions[i];
        act->setText(text);
        act->setStatusTip(filename);
        act->setVisible(true);
        ++i;
    }
    for (; i < (int)actions.size(); ++i) {
        actions[i]->setVisible(false);
    }
}
