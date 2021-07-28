
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

    std::shared_ptr<trackerboy::Instrument> getShared(int index);

    void updateChannelIcon(int index);

protected:
    virtual QIcon iconData(uint8_t id) const override;

private:
    Q_DISABLE_COPY(InstrumentListModel)

    trackerboy::InstrumentTable& table() noexcept;
    trackerboy::InstrumentTable const& table() const noexcept;

};
