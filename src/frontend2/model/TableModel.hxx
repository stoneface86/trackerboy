
#pragma once

#include "model/NameListModel.hxx"

#include <QAbstractListModel>

class TableModel final : public QAbstractListModel {

    Q_OBJECT

public:
    explicit TableModel(NameListModel *model, QObject *parent = nullptr);

    [[nodiscard]]
    Qt::ItemFlags flags(QModelIndex const &index) const override;

    [[nodiscard]]
    int rowCount(QModelIndex const &parent = QModelIndex()) const override;

    [[nodiscard]] QVariant data(QModelIndex const &index,
                                int role = Qt::DisplayRole) const override;

    void setShowEmpty(bool show);

    //
    // Gets a model index for the given table id.
    //
    [[nodiscard]] QModelIndex indexFromTable(int tableId) const;

    //
    // Gets the table id from the model index
    //
    [[nodiscard]] int id(QModelIndex const &index) const;

    [[nodiscard]] bool hasId(int id) const;

    [[nodiscard]] bool canAdd() const;

    void load();

    void add(int at);
    void remove(int at);
    void duplicate(int at);

private:
    Q_DISABLE_COPY(TableModel)

    void itemChanged(u8 id);
    void itemAdded(u8 id, QString const &name);
    void itemRemoved(u8 id);

    NameListModel *_source;
    BTableModel _backend;
};