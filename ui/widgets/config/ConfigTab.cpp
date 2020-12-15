
#include "ConfigTab.hpp"

ConfigTab::ConfigTab(Config::Category flag, QWidget *parent) :
    QWidget(parent),
    mDirtyFlag(flag),
    mDirty(false)
{

}

void ConfigTab::clean() {
    mDirty = false;
}

void ConfigTab::setDirty() {
    if (!mDirty) {
        emit dirty(mDirtyFlag);
        mDirty = true;
    }
}
