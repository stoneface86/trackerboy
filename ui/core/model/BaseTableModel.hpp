#pragma once

class ModuleDocument;

#include "trackerboy/data/Table.hpp"

#include <QAbstractListModel>
#include <QIcon>

//
// Base class for the WaveListModel and InstrumentListModel.
//
class BaseTableModel : public QAbstractListModel {

    Q_OBJECT

public:

    virtual ~BaseTableModel();

    bool canDuplicate() const;

    void reload();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    virtual QVariant data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole) const override;

    // adds a new item, if there was no items prior then this one is selected
    void add();
    // removes the given item
    void remove(int index);

    void duplicate(int index);

    QString name(int index);

    void rename(int index, const QString &name);

protected:
    BaseTableModel(ModuleDocument &document, trackerboy::BaseTable &table);

    virtual QIcon iconData(trackerboy::DataItem const& item) const = 0;

    ModuleDocument &mDocument;
    trackerboy::BaseTable &mBaseTable;

private:
    Q_DISABLE_COPY(BaseTableModel)


    void insertId(uint8_t id);


    // maps a model index -> table index
    std::vector<uint8_t> mItems;
    int mNextModelIndex;
};
