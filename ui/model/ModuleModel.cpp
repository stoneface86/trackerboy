
#include "model/ModuleModel.hpp"


ModuleModel::ModuleModel(trackerboy::Module &mod, QObject *parent) :
    mMod(mod),
    QAbstractItemModel(parent)
{
}





