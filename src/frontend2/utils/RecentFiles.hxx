
#pragma once

#include <QAction>
#include <QMenu>
#include <QString>
#include <QStringList>

#include <array>

///
/// Utility object for a group of recent files QAction.
///
struct RecentFiles {

    explicit RecentFiles() = default;

    void setup(QMenu *menu);

    void push(QString const& file);

    void setFromList(QStringList const& list);

    QStringList toList();

    QAction* separator;
    std::array<QAction*, 9> actions;


};
