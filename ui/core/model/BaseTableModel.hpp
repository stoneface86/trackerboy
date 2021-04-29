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

    // resets the model
    void reload();

    // commit all string data to the underlying data table. To be called
    // before saving the module
    void commit();

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
    BaseTableModel(ModuleDocument &document, trackerboy::BaseTable &table, QString defaultName);

    virtual QIcon iconData(uint8_t id) const = 0;

    ModuleDocument &mDocument;
    trackerboy::BaseTable &mBaseTable;

private:
    Q_DISABLE_COPY(BaseTableModel)

    // a QString copy of each name in table is stored in this model
    // this way we don't have to convert to and from std::string when
    // displaying/editing names. The conversion only occurs on reload
    // and commit
    struct ModelData {

        ModelData(uint8_t id, QString name);
        explicit ModelData(trackerboy::DataItem const& item);

        uint8_t id;
        QString name;


    };

    void insertData(ModelData const& data);

    // maps a model index -> table index
    std::vector<ModelData> mItems;

    QString const mDefaultName;
    bool mShouldCommit;
};
