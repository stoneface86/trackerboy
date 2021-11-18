#pragma once

#include "core/Module.hpp"

#include <QAbstractListModel>
#include <QIcon>

#include <string>
#include <utility>
#include <vector>

//
// Base class for the WaveListModel and InstrumentListModel.
//
class BaseTableModel : public QAbstractListModel {

    Q_OBJECT

public:

    bool canAdd() const;

    // resets the model. called when the document is reset or loaded a
    // module from a file
    void reload();

    // commit all string data to the underlying data table. To be called
    // before saving the module
    void commit();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    virtual QVariant data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole) const override;

    // adds a new item, if there was no items prior then this one is selected
    int add();
    // removes the given item
    void remove(int index);

    int duplicate(int index);

    QString name(int index);

    int id(int index);

    int lookupId(int id);

    void rename(int index, const QString &name);

    void updateChannelIcon(int index);


protected:
    BaseTableModel(Module &mod, QString defaultName, QObject *parent = nullptr);

    virtual QIcon iconData(int id) const = 0;

    virtual void commitName(int id, std::string &&name) = 0;

    //
    // Gets the source name for the id, if the id doesn't exist then
    // nullptr is returned
    //
    virtual std::string const* sourceName(int id) = 0;

    //
    // Adds a new item into the source table, the item's id is returned
    //
    virtual int sourceAdd() = 0;

    virtual int sourceDuplicate(int id) = 0;

    virtual void sourceRemove(int id) = 0;

    Module &mModule;

private:
    Q_DISABLE_COPY(BaseTableModel)

    // a QString copy of each name in table is stored in this model
    // this way we don't have to convert to and from std::string when
    // displaying/editing names. The conversion only occurs on reload
    // and commit
    using ModelData = std::pair<int, QString>;

    int insertData(ModelData const& data);

    // maps a model index -> table index
    std::vector<ModelData> mItems;

    QString const mDefaultName;
    bool mShouldCommit;

    
};
