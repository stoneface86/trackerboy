
#include "config/tabs/ConfigTab.hpp"

ConfigTab::ConfigTab(QWidget *parent) :
    QWidget(parent),
    mDirty(false)
{

}

void ConfigTab::clean() {
    mDirty = false;
}

template <Config::Category flag>
void ConfigTab::setDirty() {
    setDirty(flag);
}

void ConfigTab::setDirty(Config::Category flag) {
    if (!mDirty) {
        emit dirty(flag);
        mDirty = true;
    }
}

template void ConfigTab::setDirty<Config::CategoryAppearance>();
template void ConfigTab::setDirty<Config::CategoryGeneral>();
template void ConfigTab::setDirty<Config::CategoryKeyboard>();
template void ConfigTab::setDirty<Config::CategoryMidi>();
template void ConfigTab::setDirty<Config::CategorySound>();

