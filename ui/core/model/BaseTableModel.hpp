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

    ModuleDocument& document();

    bool canDuplicate() const;

    // resets the model
    void reload();

    // commit all string data to the underlying data table. To be called
    // before saving the module
    void commit();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    virtual QVariant data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole) const override;

    int currentIndex() const noexcept;

    // adds a new item, if there was no items prior then this one is selected
    int add();
    // removes the given item
    void remove(int index);

    int duplicate(int index);

    QString name(int index);

    uint8_t id(int index);

    int lookupId(uint8_t id);

    void rename(int index, const QString &name);

public slots:

    void setCurrentIndex(int index);


protected:
    BaseTableModel(ModuleDocument &document, trackerboy::BaseTable &table, QString defaultName);

    virtual QIcon iconData(uint8_t id) const = 0;

    ModuleDocument &mDocument;
    trackerboy::BaseTable &mBaseTable;

    int mCurrentIndex;

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

    int insertData(ModelData const& data);

    // maps a model index -> table index
    std::vector<ModelData> mItems;

    QString const mDefaultName;
    bool mShouldCommit;

    
};
