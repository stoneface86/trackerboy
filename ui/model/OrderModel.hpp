
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
    
    //
    // All track ids in the given selection are incremented by 1
    //
    void incrementSelection(QItemSelection const &selection);

    //
    // All track ids in the given selection are decremented by 1
    //
    void decrementSelection(QItemSelection const &selection);
    
    void select(int row, int track);
    
    //
    // Set the OrderActions struct, the model will enable/disable actions
    // based on the underlying data
    //
    void setActions(OrderActions actions);
    
    //
    // Change the model's order data. Should only be called by SongListModel.
    //
    void setOrder(std::vector<trackerboy::Order> *order);

    //
    // All items in the given selection are set to the given track id
    //
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
    // the selected pattern has changed
    void currentPatternChanged(int pattern);

    // the selected track/channel has changed
    void currentTrackChanged(int track);

    // same as the above signals but as a QModelIndex
    void currentIndexChanged(const QModelIndex &index);

    
    void patternsChanged();

public slots:
    void insert();

    void remove();

    void duplicate();

    void moveUp();

    void moveDown();

    void selectPattern(int pattern);

    void selectTrack(int track);

private:
    enum class ModifyMode {
        inc,
        dec,
        set
    };

    template <ModifyMode mode>
    void modifySelection(QItemSelection const &selection, uint8_t option = 0);

    template <ModifyMode mode>
    void modifyCell(uint8_t &cell, uint8_t value);

    void updateActions();

    ModuleDocument &mDocument;

    std::vector<trackerboy::Order> *mOrder;

    OrderActions mActions;

    int mCurrentRow;
    int mCurrentTrack;

    bool mCanSelect;
    
};
