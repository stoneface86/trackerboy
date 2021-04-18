
#pragma once

#include "core/model/BaseTableModel.hpp"

#include <QIcon>

#include <array>

//
// Model implementation for a trackerboy::InstrumentTable
//
class InstrumentListModel : public BaseTableModel {

    Q_OBJECT

public:
    InstrumentListModel(ModuleDocument &document);

protected:
    virtual QIcon iconData(trackerboy::DataItem const& item) const override;

private:
    Q_DISABLE_COPY(InstrumentListModel)

};
