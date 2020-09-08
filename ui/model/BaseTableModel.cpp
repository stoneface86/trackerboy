
#include "model/BaseTableModel.hpp"


BaseTableModel::BaseTableModel(trackerboy::BaseTable &table, QObject *parent) :
    mBaseTable(table),
    mEnabled(true),
    QAbstractListModel(parent)
{
}

int BaseTableModel::rowCount(const QModelIndex &parent) const {
    (void)parent;
    return (mEnabled) ? mBaseTable.size() : 0;
}

QVariant BaseTableModel::data(const QModelIndex &index, int role) const {
    if (mEnabled) {
        if (role == Qt::DisplayRole) {
            auto *item = mBaseTable.getFromOrder(index.row());
            QString str("%1 - %2");
            return QVariant(str.arg(QString::number(item->id()), QString::fromStdString(item->name())));
        } else if (role == Qt::DecorationRole) {
            return iconData(index);
        }
    }

    return QVariant();
}

void BaseTableModel::addItem() {
    int row = mBaseTable.size();
    beginInsertRows(QModelIndex(), row, row);
    mBaseTable.insertItem();
    endInsertRows();
}

QString BaseTableModel::name(int index) {
    return QString::fromStdString(mBaseTable.getFromOrder(index)->name());
}

void BaseTableModel::setEnabled(bool enable) {
    if (enable != mEnabled) {
        beginResetModel();
        mEnabled = enable;
        endResetModel();
    }
}

void BaseTableModel::setName(int index, QString name) {
    mBaseTable.getFromOrder(index)->setName(name.toStdString());
    emit dataChanged(createIndex(index, 0, nullptr), createIndex(index, 0, nullptr), { Qt::DisplayRole });
}
