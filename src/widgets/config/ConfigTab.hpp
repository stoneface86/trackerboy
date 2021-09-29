
#pragma once

#include "core/Config.hpp"

#include <QWidget>


//
// Base class for a configuration tab, or the underlying widget for a tab in a QTabWidget.
// When the user modifies the configuration, the tab is marked as "dirty"
//
class ConfigTab : public QWidget {
    
    Q_OBJECT

public:
    ConfigTab(Config::Category flag, QWidget *parent = nullptr);

    void clean();

signals:
    void dirty(Config::Category flag);

protected slots:
    void setDirty();


private:
    Q_DISABLE_COPY(ConfigTab)

    Config::Category const mDirtyFlag;
    bool mDirty;

};
