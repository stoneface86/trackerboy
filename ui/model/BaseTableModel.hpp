#pragma once

#include "model/BaseModel.hpp"
#include "trackerboy/data/Table.hpp"


class BaseTableModel : public BaseModel {
    Q_OBJECT

public:

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole) const override;

    void addItem();

    virtual QString name() override;

protected:
    BaseTableModel(trackerboy::BaseTable &table, QObject *parent = nullptr);

    virtual QVariant iconData(const QModelIndex &index) const = 0;

    virtual void setNameInData(QString name) override;

    trackerboy::BaseTable &mBaseTable;


};
