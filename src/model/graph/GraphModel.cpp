
#include "model/graph/GraphModel.hpp"
#include "verdigris/wobjectimpl.h"

W_OBJECT_IMPL(GraphModel)

GraphModel::GraphModel(Module &mod, QObject *parent) :
    QObject(parent),
    mModule(mod)
{
}
