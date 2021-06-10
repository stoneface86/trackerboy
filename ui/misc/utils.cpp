
#include "misc/utils.hpp"

#include <QCoreApplication>
#include <QStack>
#include <QPair>

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

void expandRecursively(QTreeView &view, QModelIndex const& index, int depth) {

    if (depth < -1) {
        return;
    }
    // expand the first index
    view.expand(index);
    if (depth == 0) {
        return;
    }

    auto model = view.model();


    QStack<QPair<QModelIndex, int>> parents;
    parents.push({index, 0});
    while (!parents.isEmpty()) {
        const QPair<QModelIndex, int> elem = parents.pop();
        const QModelIndex &parent = elem.first;
        const int curDepth = elem.second;
        const int rowCount = model->rowCount(parent);
        for (int row = 0; row < rowCount; ++row) {
            const QModelIndex child = model->index(row, 0, parent);
            view.expand(child);
            if (depth == -1 || curDepth + 1 < depth) {
                parents.push({child, curDepth + 1});
            }
        }
    }
}
