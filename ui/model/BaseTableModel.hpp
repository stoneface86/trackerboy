#pragma once

#include <QAbstractListModel>
#include "trackerboy/data/Table.hpp"


class BaseTableModel : public QAbstractListModel {
    Q_OBJECT

public:

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole) const override;

    void addItem();

    QString name(int index);

    void setName(int index, QString name);

protected:
    BaseTableModel(trackerboy::BaseTable &table, QObject *parent = nullptr);

    virtual QVariant iconData(const QModelIndex &index) const = 0;
    virtual void insertItemInTable() = 0;
    virtual void removeItemInTable(uint8_t index) = 0;


    trackerboy::BaseTable &mBaseTable;

};
