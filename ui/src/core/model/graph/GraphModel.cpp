
#include "core/model/graph/GraphModel.hpp"

GraphModel::GraphModel(Module &mod, QObject *parent) :
    QObject(parent),
    mModule(mod)
{
}
