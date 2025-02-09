
#pragma once

#include "config/Config.hpp"

#include <QWidget>


//
// Base class for a configuration tab, or the underlying widget for a tab in a QTabWidget.
// When the user modifies the configuration, the tab is marked as "dirty"
//
class ConfigTab : public QWidget {
    
    Q_OBJECT

public:
    ConfigTab(QWidget *parent = nullptr);

    void clean();

signals:
    void dirty(Config::Category flag);

protected:

    template <Config::Category flag>
    void setDirty();

    void setDirty(Config::Category flag);


private:
    Q_DISABLE_COPY(ConfigTab)

    bool mDirty;

};
