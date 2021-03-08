#pragma once

#include "core/model/BaseModel.hpp"

#include "trackerboy/data/Table.hpp"


//
// Base class for the WaveListModel and InstrumentListModel.
//
class BaseTableModel : public BaseModel {
    Q_OBJECT

public:

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // convert a table id to its model index
    int idToModel(uint8_t id);

    QVariant data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole) const override;

    virtual QString nameAt(int index) override;

protected:
    BaseTableModel(ModuleDocument &document, trackerboy::BaseTable &table);

    virtual void dataAdd() override;

    virtual void dataRemove(int row) override;

    virtual void dataDuplicate(int row) override;

    virtual QVariant iconData(const QModelIndex &index) const = 0;

    virtual void dataRename(int index, const QString &name) override;

    virtual bool canAdd() override;

    virtual bool canRemove() override;

    virtual int nextIndex() override;

    trackerboy::BaseTable &mBaseTable;

private:
    Q_DISABLE_COPY(BaseTableModel)

};
