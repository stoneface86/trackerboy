
#pragma once

#include "core/model/ModuleDocument.hpp"

#include <QAbstractItemModel>
#include <QVector>

#include <variant>

//
// Model implementation to be used in a QTreeView. Shows all open modules
//
class ModuleModel : public QAbstractItemModel {

    Q_OBJECT

public:

    explicit ModuleModel(QObject *parent = nullptr);

    virtual Qt::ItemFlags flags(QModelIndex const& index) const override;

    virtual QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;

    virtual bool hasChildren(QModelIndex const& index) const override;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    virtual QModelIndex index(int row, int column, QModelIndex const& parent = QModelIndex()) const override;

    virtual QModelIndex parent(QModelIndex const& index) const override;

    virtual int rowCount(QModelIndex const& parent = QModelIndex()) const override;

    virtual int columnCount(QModelIndex const& parent = QModelIndex()) const override;

    QModelIndex addDocument(ModuleDocument *doc);

    void removeDocument(ModuleDocument *doc);
    
    QVector<ModuleDocument*> const& documents() const noexcept;

private:


    QVector<ModuleDocument*> mDocuments;
};
