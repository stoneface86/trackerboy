
#pragma once

#include "model/NameListModel.hxx"

#include <QAbstractListModel>

class TableModel : public QAbstractListModel {

    Q_OBJECT

public:
    explicit TableModel(NameListModel *model, QObject *parent = nullptr);

    virtual Qt::ItemFlags flags(QModelIndex const &index) const override;

    virtual int
    rowCount(QModelIndex const &index = QModelIndex()) const override;

    virtual QVariant data(QModelIndex const &index,
                          int role = Qt::DisplayRole) const override;

    void load();

private:
    Q_DISABLE_COPY(TableModel)

    static constexpr u8 cEmptyId = (u8)(B::TableCap);

    NameListModel *_source;
    std::array<u8, B::TableCap> _idMap;
    bool _showEmpty;
};