
#pragma once

#include <QAbstractItemModel>

#include "trackerboy/data/Module.hpp"

class ModuleModel : public QAbstractItemModel {

public:
    ModuleModel(trackerboy::Module &mMod, QObject *parent = nullptr);


private:

    trackerboy::Module &mMod;

};
