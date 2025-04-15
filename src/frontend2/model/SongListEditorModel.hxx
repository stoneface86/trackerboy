
#pragma once

#include "model/SongListModel.hxx"

#include <QAbstractItemModel>
#include <QList>

///
/// Model subclass for editing the song list. The model has four columns:
/// - The song number
/// - The song's name
/// - A checkbox to remove the song from the list
/// - Description of the pending change to the song
///
/// This model allows for editing the song list out-of-place.
///
class SongListEditorModel final : public QAbstractItemModel {

    Q_OBJECT

public:
    explicit SongListEditorModel(SongListModel *sourceModel,
                                 QObject *parent = nullptr);
    virtual ~SongListEditorModel() = default;

    virtual Qt::ItemFlags flags(QModelIndex const &index) const override final;

    virtual QModelIndex
    index(int row, int column = 0,
          QModelIndex const &parent = {}) const override final;

    virtual QModelIndex
    parent(QModelIndex const &index = {}) const override final;

    virtual int
    columnCount(QModelIndex const &parent = {}) const override final;

    virtual int rowCount(QModelIndex const &parent = {}) const override final;
    virtual QVariant data(QModelIndex const &index,
                          int role = Qt::DisplayRole) const override final;

    virtual QVariant
    headerData(int section, Qt::Orientation orientation,
               int role = Qt::DisplayRole) const override final;

    virtual bool setData(QModelIndex const &index, QVariant const &value,
                         int role = Qt::EditRole) override final;

    virtual Qt::DropActions supportedDropActions() const override final;

    virtual bool dropMimeData(QMimeData const *data, Qt::DropAction action,
                              int row, int column,
                              QModelIndex const &parent = {}) override final;

    virtual QMimeData *
    mimeData(QModelIndexList const &indexes) const override final;

    virtual QStringList mimeTypes() const override final;

    void add();
    void duplicate(int index);
    void moveUp(int index);
    void moveDown(int index);
    void reset();
    void apply(Document &doc);

private:
    Q_DISABLE_COPY(SongListEditorModel)

    void setListFromSource();
    int countRemoved() const;

    enum Columns { colNumber, colName, colRemove, colStatus, colCount };

    enum ItemAction { itemKeep, itemNew, itemRemove, itemDuplicate };

    struct Item {
        qint8 sourceId;
        qint8 action;
        bool nameModified;
        QString name;
    };

    SongListModel *mSource;
    QList<Item> mItems;
};