
#include "model/BaseTableModel.hpp"

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

int BaseTableModel::dataAdd() {
    int row = mBaseTable.nextModelId();

    beginInsertRows(QModelIndex(), row, row);
    
    mDocument.lock();
    mBaseTable.insertItem();
    mDocument.unlock();

    endInsertRows();

    return row;
}

int BaseTableModel::dataRemove() {
    uint8_t id = mBaseTable.lookup(static_cast<uint8_t>(mCurrentIndex));

    int index = mCurrentIndex;
    beginRemoveRows(QModelIndex(), index, index);

    mDocument.lock();
    mBaseTable.remove(id);
    mDocument.unlock();

    endRemoveRows();
    return mBaseTable.size() == index ? index - 1 : index;
}

int BaseTableModel::dataDuplicate() {
    int row = mBaseTable.nextModelId();

    uint8_t index = static_cast<uint8_t>(mCurrentIndex);
    beginInsertRows(QModelIndex(), row, row);

    mDocument.lock();
    mBaseTable.duplicate(mBaseTable.lookup(index));
    mDocument.unlock();

    endInsertRows();

    return row;
}

bool BaseTableModel::canDuplicate() {
    return mBaseTable.size() < trackerboy::BaseTable::MAX_SIZE;
}

bool BaseTableModel::canRemove() {
    return mBaseTable.size() > 0;
}

QString BaseTableModel::name() {
    return QString::fromStdString(mBaseTable.getFromOrder(static_cast<uint8_t>(mCurrentIndex))->name());
}

void BaseTableModel::dataRename(const QString &name) {
    mBaseTable.getFromOrder(static_cast<uint8_t>(mCurrentIndex))->setName(name.toStdString());
}
