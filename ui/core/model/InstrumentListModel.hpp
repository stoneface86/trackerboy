
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
    InstrumentListModel(Module &mod, QObject *parent = nullptr);

    std::shared_ptr<trackerboy::Instrument> currentInstrument();

    void updateChannelIcon();

protected:
    virtual QIcon iconData(uint8_t id) const override;

private:
    Q_DISABLE_COPY(InstrumentListModel)

};
