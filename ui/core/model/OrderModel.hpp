
#pragma once

#include "core/model/ModuleDocument.hpp"

#include "trackerboy/data/Order.hpp"

#include <QAbstractTableModel>
#include <QColor>
#include <QItemSelection>
#include <QModelIndex>

#include <vector>

//
// Model class for the song order editor. The song order is a "table"
// with 4 columns being track ids (1 for each channel). Each row in the
// model is a pattern in the song.
//
class OrderModel : public QAbstractTableModel {

    Q_OBJECT

public:

    explicit OrderModel(ModuleDocument &document, QObject *parent = nullptr);
    ~OrderModel() = default;

    void reload();

    uint8_t currentPattern();

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
    // All items in the given selection are set to the given track id
    //
    void setSelection(QItemSelection const &selection, uint8_t id);

    void setRowColor(QColor const& color);

    // model implementation

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    //bool insertRows(int row, int count, QModelIndex const &parent = QModelIndex()) override;

    //bool removeRows(int row, int count, QModelIndex const &parent = QModelIndex()) override;

signals:
    // the selected pattern has changed
    void currentPatternChanged(int pattern);

    // the selected track/channel has changed
    void currentTrackChanged(int track);

    // same as the above signals but as a QModelIndex
    void currentIndexChanged(const QModelIndex &index);

    
    void patternsChanged();

    // connect these signals to QAction::setEnabled
    void canInsert(bool state);
    void canRemove(bool state);
    void canMoveUp(bool state);
    void canMoveDown(bool state);

public slots:
    void insert();

    void remove();

    void duplicate();

    void moveUp();

    void moveDown();

    void selectPattern(int pattern);

    void selectTrack(int track);

    void setPatternCount(int count);

private:
    Q_DISABLE_COPY(OrderModel)

    // QUndoCommand classes
    friend class OrderModelCommand;
    friend class OrderModifyCommand;
    friend class OrderSwapCommand;
    friend class OrderInsertCommand;
    friend class OrderRemoveCommand;
    friend class OrderDuplicateCommand;


    enum class ModifyMode {
        inc,
        dec,
        set
    };

    void doSelectPattern(uint8_t pattern);
    void doSelectTrack(uint8_t track);

    template <ModifyMode mode>
    void modifySelection(QItemSelection const &selection, uint8_t option = 0);

    template <ModifyMode mode>
    void modifyCell(QUndoStack &stack, uint8_t pattern, uint8_t track, uint8_t option = 0);

    // methods called by command classes

    void cmdInsertRows(uint8_t row, uint8_t count, trackerboy::OrderRow *rowdata = nullptr);

    void cmdRemoveRows(uint8_t row, uint8_t count);

    void cmdSwapRows(uint8_t from, uint8_t to);

    

    ModuleDocument &mDocument;
    QColor mRowColor;

    trackerboy::Order &mOrder;
    //std::vector<trackerboy::Order> *mOrder;

    uint8_t mCurrentRow;
    uint8_t mCurrentTrack;

    bool mCanSelect;
    
};
