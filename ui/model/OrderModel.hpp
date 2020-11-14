
#pragma once

#include "trackerboy/data/Order.hpp"

#include "model/ModuleDocument.hpp"

#include <QAbstractTableModel>
#include <QItemSelection>
#include <QModelIndex>

#include <vector>

class OrderModel : public QAbstractTableModel {

    Q_OBJECT

public:
    explicit OrderModel(ModuleDocument &document, QObject *parent = nullptr);
    virtual ~OrderModel() = default;
    
    void setOrder(std::vector<trackerboy::Order> *order);

    void incrementSelection(QItemSelection const &selection);

    void decrementSelection(QItemSelection const &selection);

    void setSelection(QItemSelection const &selection, uint8_t id);


    // model implementation

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

private:
    enum class ModifyMode {
        incdec,
        set
    };

    template <ModifyMode mode>
    void modifySelection(int option, QItemSelection const &selection);

    ModuleDocument &mDocument;

    std::vector<trackerboy::Order> *mOrder;
    
};
