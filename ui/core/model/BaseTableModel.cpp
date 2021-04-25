
#include "core/model/BaseTableModel.hpp"
#include "misc/IconManager.hpp"

#include "core/model/ModuleDocument.hpp"

#include <QStringBuilder>

BaseTableModel::BaseTableModel(ModuleDocument &document, trackerboy::BaseTable &table) :
    mDocument(document),
    mBaseTable(table),
    mItems(),
    mNextModelIndex(0)
{
}

BaseTableModel::~BaseTableModel() {

}

bool BaseTableModel::canDuplicate() const {
    return mBaseTable.size() != trackerboy::BaseTable::MAX_SIZE;
}

void BaseTableModel::reload() {
    beginResetModel();

    mItems.clear();
    for (uint8_t id = 0; id != trackerboy::BaseTable::MAX_SIZE; ++id) {
        if (mBaseTable.get(id) != nullptr) {
            mItems.push_back(id);
        }
    }

    endResetModel();
}

int BaseTableModel::rowCount(const QModelIndex &parent) const {
    (void)parent;
    // row count is fixed by the maximum number of items in trackerboy::BaseTable
    return (int)mItems.size();
}

QVariant BaseTableModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        auto item = mBaseTable.get(mItems[index.row()]);
        Q_ASSERT(item != nullptr);
        // <id> - [name]
        return QString::number(item->id(), 16).toUpper().rightJustified(2, '0') %
            QStringLiteral(" - ") %
            QString::fromStdString(item->name());
    } else if (role == Qt::DecorationRole) {
        auto item = mBaseTable.get(mItems[index.row()]);
        return iconData(*item);
    }

    return QVariant();
}

QString BaseTableModel::name(int index) {
    auto item = mBaseTable.get(mItems[index]);
    return QString::fromStdString(item->name());
}

void BaseTableModel::add() {
    Q_ASSERT(mBaseTable.size() < trackerboy::BaseTable::MAX_SIZE);

    uint8_t id;
    {
        auto ctx = mDocument.beginEdit();
        auto &item = mBaseTable.insert();
        id = item.id();
    }

    insertId(id);

}

void BaseTableModel::remove(int index) {
    beginRemoveRows(QModelIndex(), index, index);
    auto iter = mItems.begin() + index;
    {
        auto ctx = mDocument.beginEdit();
        mBaseTable.remove(*iter);
    }

    mItems.erase(iter);
    endRemoveRows();
    
}


void BaseTableModel::duplicate(int index) {
    
    uint8_t id;
    {
        auto ctx = mDocument.beginEdit();
        id = mBaseTable.duplicate(mItems[index]).id();
    }
    insertId(id);

}

void BaseTableModel::rename(int index, const QString &name) {

    auto item = mBaseTable.get(mItems[index]);
    Q_ASSERT(item != nullptr);

    {
        auto ctx = mDocument.beginEdit();
        item->setName(name.toStdString());
    }

    auto index_ = createIndex(index, 0, nullptr);
    emit dataChanged(index_, index_, { Qt::DisplayRole });
}


void BaseTableModel::insertId(uint8_t id) {
    auto itemCount = mItems.size();
    if (itemCount == 0 || *mItems.rbegin() < id) {
        // shortcut, put the id at the end
        beginInsertRows(QModelIndex(), itemCount, itemCount);
        mItems.push_back(id);
        endInsertRows();

    } else {
        // search and insert
        auto begin = mItems.begin();
        auto iter = std::upper_bound(begin, mItems.end(), id);

        int row = iter - begin;
        beginInsertRows(QModelIndex(), row, row);
        mItems.insert(iter, id);
        endInsertRows();
    }
}

