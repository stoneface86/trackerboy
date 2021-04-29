
#include "core/model/BaseTableModel.hpp"
#include "misc/IconManager.hpp"

#include "core/model/ModuleDocument.hpp"

#include <QStringBuilder>

BaseTableModel::ModelData::ModelData(uint8_t id, QString name) :
    id(id),
    name(name)
{
}

BaseTableModel::ModelData::ModelData(trackerboy::DataItem const& item) :
    id(item.id()),
    name(QString::fromStdString(item.name()))
{
}

BaseTableModel::BaseTableModel(ModuleDocument &document, trackerboy::BaseTable &table, QString defaultName) :
    mDocument(document),
    mBaseTable(table),
    mItems(),
    mDefaultName(defaultName),
    mShouldCommit(false)
{
}

BaseTableModel::~BaseTableModel() {

}

bool BaseTableModel::canDuplicate() const {
    return mBaseTable.size() != trackerboy::BaseTable::MAX_SIZE;
}

void BaseTableModel::commit() {
    // set the names of all items in the table
    if (mShouldCommit) {
        for (auto &data : mItems) {
            auto tableItem = mBaseTable.get(data.id);
            tableItem->setName(data.name.toStdString());
        }
        mShouldCommit = false;
    }
}

void BaseTableModel::reload() {
    beginResetModel();

    mItems.clear();
    for (uint8_t id = 0; id != trackerboy::BaseTable::MAX_SIZE; ++id) {
        auto item = mBaseTable.get(id);
        if (item != nullptr) {
            mItems.emplace_back(*item);
        }
    }

    endResetModel();
}

int BaseTableModel::rowCount(const QModelIndex &parent) const {
    (void)parent;
    return (int)mItems.size();
}

QVariant BaseTableModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole || role == Qt::DecorationRole) {

        auto const& modelItem = mItems[index.row()];

        if (role == Qt::DisplayRole) {
            // <id> - [name]
            return QString::number(modelItem.id, 16).toUpper().rightJustified(2, '0') %
                QStringLiteral(" - ") % modelItem.name;
        } else {
            // decoration role
            return iconData(modelItem.id);
        }
    }

    return QVariant();
}

QString BaseTableModel::name(int index) {
    return mItems[index].name;
}

void BaseTableModel::add() {
    Q_ASSERT(mBaseTable.size() < trackerboy::BaseTable::MAX_SIZE);

    uint8_t id;
    {
        auto ctx = mDocument.beginEdit();
        auto &item = mBaseTable.insert();
        id = item.id();
    }
    ModelData data(id, mDefaultName);
    insertData(data);

}

void BaseTableModel::remove(int index) {
    beginRemoveRows(QModelIndex(), index, index);
    auto iter = mItems.begin() + index;
    {
        auto ctx = mDocument.beginEdit();
        mBaseTable.remove(iter->id);
    }

    mItems.erase(iter);
    endRemoveRows();
    
}


void BaseTableModel::duplicate(int index) {

    auto const& dataToCopy = mItems[index];
    uint8_t id;
    {
        auto ctx = mDocument.beginEdit();
        id = mBaseTable.duplicate(dataToCopy.id).id();
    }

    ModelData data(id, dataToCopy.name);
    insertData(data);

}

void BaseTableModel::rename(int index, const QString &name) {
    mShouldCommit = true;

    mItems[index].name = name;
    mDocument.makeDirty();

    auto index_ = createIndex(index, 0, nullptr);
    emit dataChanged(index_, index_, { Qt::DisplayRole });
}


void BaseTableModel::insertData(ModelData const& data) {
    mShouldCommit = true;

    auto itemCount = mItems.size();
    if (itemCount == 0 || mItems.rbegin()->id < data.id) {
        // shortcut, put the id at the end
        beginInsertRows(QModelIndex(), (int)itemCount, (int)itemCount);
        mItems.push_back(data);
        endInsertRows();

    } else {
        // search and insert
        auto begin = mItems.begin();
        auto iter = std::upper_bound(begin, mItems.end(), data.id,
            [](uint8_t id, ModelData const& data) {
                return id < data.id;
            });

        int row = (int)(iter - begin);
        beginInsertRows(QModelIndex(), row, row);
        mItems.insert(iter, data);
        endInsertRows();
    }
}

