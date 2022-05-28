
#pragma once

#include "core/Module.hpp"
#include "verdigris/wobjectdefs.h"

#include <QObject>

#include <cstdint>

//
// Model class for graph data, waveforms and sequences. To be used in a
// GraphEdit
//
class GraphModel : public QObject {

    W_OBJECT(GraphModel)

public:

    // 8-bit integer data type
    using DataType = int8_t;

    virtual int count() = 0;

    virtual DataType dataAt(int i) = 0;

    virtual void setData(int i, DataType data) = 0;

//signals:
    void countChanged(int count) W_SIGNAL(countChanged, count)
    void dataChanged() W_SIGNAL(dataChanged)

protected:
    explicit GraphModel(Module &mod, QObject *parent = nullptr);

    Module &mModule;

private:

};
