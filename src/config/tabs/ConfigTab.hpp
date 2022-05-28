
#pragma once

#include "config/Config.hpp"
#include "verdigris/wobjectdefs.h"

#include <QWidget>


//
// Base class for a configuration tab, or the underlying widget for a tab in a QTabWidget.
// When the user modifies the configuration, the tab is marked as "dirty"
//
class ConfigTab : public QWidget {
    
    W_OBJECT(ConfigTab)

public:
    ConfigTab(QWidget *parent = nullptr);

    void clean();

//signals:
    void dirty(Config::Category flag) W_SIGNAL(dirty, flag)

protected:

    template <Config::Category flag>
    void setDirty();

    void setDirty(Config::Category flag);


private:
    Q_DISABLE_COPY(ConfigTab)

    bool mDirty;

};
