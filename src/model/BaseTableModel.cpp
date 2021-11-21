
#include "model/BaseTableModel.hpp"

#include <QStringBuilder>

BaseTableModel::BaseTableModel(Module &mod, QString defaultName, QObject *parent) :
    QAbstractListModel(parent),
    mModule(mod),
    mItems(),
    mDefaultName(defaultName),
    mShouldCommit(false)
{
    connect(&mod, &Module::reloaded, this, &BaseTableModel::reload);
    connect(&mod, &Module::aboutToSave, this, &BaseTableModel::commit);
}

bool BaseTableModel::canAdd() const {
    return mItems.size() != trackerboy::InstrumentTable::MAX_SIZE;
}

void BaseTableModel::commit() {
    // set the names of all items in the table
    if (mShouldCommit) {
        auto ctx = mModule.permanentEdit();
        for (auto &data : mItems) {
            commitName(data.first, data.second.toStdString());
        }
        mShouldCommit = false;
    }
}

void BaseTableModel::reload() {
    beginResetModel();

    mItems.clear();
    for (size_t id = 0; id != trackerboy::InstrumentTable::MAX_SIZE; ++id) {
        auto str = sourceName(id);
        if (str) {
            mItems.emplace_back(id, QString::fromStdString(*str));
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
            QString str = QString::number(modelItem.first, 16).toUpper().rightJustified(2, '0') %
                QStringLiteral(" - ") % modelItem.second;
            return str;
        } else {
            // decoration role
            return iconData(modelItem.first);
        }
    }

    return QVariant();
}

int BaseTableModel::id(int index) {
    return mItems[index].first;
}

int BaseTableModel::lookupId(int id) {
    auto item = std::find_if(mItems.begin(), mItems.end(),
        [id](ModelData &data) {
            return data.first == id;
        });
    if (item == mItems.end()) {
        return -1;
    } else {
        return (int)(item - mItems.begin());
    }
}

QString BaseTableModel::name(int index) {
    return mItems[index].second;
}

int BaseTableModel::add() {
    Q_ASSERT(canAdd());

    int id;
    {
        auto ctx = mModule.permanentEdit();
        id = sourceAdd();
    }
    return insertData(ModelData(id, mDefaultName));

}

void BaseTableModel::remove(int index) {
    beginRemoveRows(QModelIndex(), index, index);
    auto iter = mItems.begin() + index;
    {
        auto ctx = mModule.permanentEdit();
        sourceRemove(iter->first);
    }

    mItems.erase(iter);
    endRemoveRows();
    
}


int BaseTableModel::duplicate(int index) {

    auto const& dataToCopy = mItems[index];
    int id;
    {
        auto ctx = mModule.permanentEdit();
        id = sourceDuplicate(dataToCopy.first);
    }

    return insertData(ModelData(id, dataToCopy.second));

}

void BaseTableModel::rename(int index, const QString &name) {
    mShouldCommit = true;

    mItems[index].second = name;
    mModule.makeDirty();

    auto index_ = createIndex(index, 0, nullptr);
    emit dataChanged(index_, index_, { Qt::DisplayRole });
}

void BaseTableModel::updateChannelIcon(int index) {
    auto modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, { Qt::DecorationRole });
}


int BaseTableModel::insertData(ModelData const& data) {
    mShouldCommit = true;

    auto itemCount = mItems.size();
    if (itemCount == 0 || mItems.rbegin()->first < data.first) {
        // shortcut, put the id at the end
        beginInsertRows(QModelIndex(), (int)itemCount, (int)itemCount);
        mItems.push_back(data);
        endInsertRows();
        return (int)itemCount;
    } else {
        // search and insert
        auto begin = mItems.begin();
        auto iter = std::upper_bound(begin, mItems.end(), data.first,
            [](uint8_t id, ModelData const& data) {
                return id < data.first;
            });

        int row = (int)(iter - begin);
        beginInsertRows(QModelIndex(), row, row);
        mItems.insert(iter, data);
        endInsertRows();

        return row;
    }
}

