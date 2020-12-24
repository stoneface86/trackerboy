
#include "core/model/BaseTableModel.hpp"

BaseTableModel::BaseTableModel(ModuleDocument &document, trackerboy::BaseTable &table) :
    mBaseTable(table),
    //mEnabled(true),
    BaseModel(document)
{
}

int BaseTableModel::rowCount(const QModelIndex &parent) const {
    (void)parent;
    return static_cast<int>(mBaseTable.size());
}

int BaseTableModel::idToModel(uint8_t id) {
    auto item = mBaseTable.get(id);
    if (item == nullptr) {
        return -1;
    } else {
        int index = 0;
        for (auto item : mBaseTable) {
            if (item == id) {
                break;
            }
            ++index;
        }
        return index;
    }
}

QVariant BaseTableModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        auto *item = mBaseTable.getFromOrder(static_cast<uint8_t>(index.row()));
        QString str("%1 - %2");
        return QVariant(str.arg(
            QString::number(item->id(), 16).toUpper().rightJustified(2, '0'), 
            QString::fromStdString(item->name())
        ));
    } else if (role == Qt::DecorationRole) {
        return iconData(index);
    }

    return QVariant();
}

void BaseTableModel::dataAdd() {
    mBaseTable.insertItem();
}

void BaseTableModel::dataRemove(int row) {
    mBaseTable.remove(mBaseTable.lookup(static_cast<uint8_t>(row)));
}

void BaseTableModel::dataDuplicate(int row) {
    mBaseTable.duplicate(mBaseTable.lookup(static_cast<uint8_t>(row)));
}

bool BaseTableModel::canAdd() {
    return mBaseTable.size() < trackerboy::BaseTable::MAX_SIZE;
}

bool BaseTableModel::canRemove() {
    return mBaseTable.size() > 0;
}

int BaseTableModel::nextIndex() {
    return mBaseTable.nextModelId();
}

QString BaseTableModel::nameAt(int index) {
    return QString::fromStdString(mBaseTable.getFromOrder(static_cast<uint8_t>(index))->name());
}

void BaseTableModel::dataRename(int index, const QString &name) {
    mBaseTable.getFromOrder(static_cast<uint8_t>(index))->setName(name.toStdString());
}
