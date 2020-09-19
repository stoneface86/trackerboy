#pragma once

#include "model/BaseModel.hpp"
#include "trackerboy/data/Table.hpp"


class BaseTableModel : public BaseModel {
    Q_OBJECT

public:

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole) const override;

    //void addItem();

    virtual QString name() override;

protected:
    BaseTableModel(ModuleDocument &document, trackerboy::BaseTable &table);

    virtual int dataAdd() override;

    virtual int dataRemove() override;

    virtual int dataDuplicate() override;

    virtual QVariant iconData(const QModelIndex &index) const = 0;

    virtual void dataRename(const QString &name) override;

    virtual bool canDuplicate() override;

    virtual bool canRemove() override;

    trackerboy::BaseTable &mBaseTable;


};
