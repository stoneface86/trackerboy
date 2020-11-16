
#pragma once

#include "trackerboy/data/Order.hpp"

#include "model/ModuleDocument.hpp"
#include "misc/OrderActions.hpp"

#include <QAbstractTableModel>
#include <QItemSelection>
#include <QModelIndex>

#include <vector>

class OrderModel : public QAbstractTableModel {

    Q_OBJECT

public:
    explicit OrderModel(ModuleDocument &document, QObject *parent = nullptr);
    ~OrderModel() = default;
    
    void incrementSelection(QItemSelection const &selection);

    void decrementSelection(QItemSelection const &selection);
    
    void select(int row, int track);
    
    void setActions(OrderActions actions);
    
    void setOrder(std::vector<trackerboy::Order> *order);

    void setSelection(QItemSelection const &selection, uint8_t id);

    // model implementation

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    bool insertRows(int row, int count, QModelIndex const &parent = QModelIndex()) override;

    bool removeRows(int row, int count, QModelIndex const &parent = QModelIndex()) override;

signals:
    void patternChanged(int pattern);

    void trackChanged(int track);

public slots:
    void insert();

    void remove();

    void duplicate();

    void moveUp();

    void moveDown();


private:
    enum class ModifyMode {
        incdec,
        set
    };

    template <ModifyMode mode>
    void modifySelection(uint8_t option, QItemSelection const &selection);

    void _insert(trackerboy::Order order);

    ModuleDocument &mDocument;

    std::vector<trackerboy::Order> *mOrder;

    OrderActions mActions;

    int mCurrentRow;
    int mCurrentTrack;
    
};
