
#pragma once
#include "core/model/BaseTableModel.hpp"

#include "trackerboy/data/Instrument.hpp"
#include "trackerboy/data/Table.hpp"
#include "trackerboy/data/Waveform.hpp"

template <class T>
class TableModel : public BaseTableModel {

public:

    TableModel(Module &mod, QObject *parent = nullptr);

    std::shared_ptr<T> getShared(int index);

protected:

    virtual QIcon iconData(int id) const override;

    virtual void commitName(int id, std::string &&name) override;

    virtual std::string const* sourceName(int id) override;

    virtual int sourceAdd() override;

    virtual int sourceDuplicate(int id) override;

    virtual void sourceRemove(int id) override;

private:

    trackerboy::Table<T>& source();
    trackerboy::Table<T> const& source() const;

};


using InstrumentListModel = TableModel<trackerboy::Instrument>;
using WaveListModel = TableModel<trackerboy::Waveform>;
