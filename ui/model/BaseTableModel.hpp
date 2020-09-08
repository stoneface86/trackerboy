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

    // disable or enable the model, used when opening files or clearing the module
    void setEnabled(bool enable);

    void setName(int index, QString name);

protected:
    BaseTableModel(trackerboy::BaseTable &table, QObject *parent = nullptr);

    virtual QVariant iconData(const QModelIndex &index) const = 0;


    trackerboy::BaseTable &mBaseTable;

    bool mEnabled;

};
