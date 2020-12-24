
#pragma once

#include "core/Config.hpp"

#include <QWidget>


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
    Config::Category const mDirtyFlag;
    bool mDirty;

};
