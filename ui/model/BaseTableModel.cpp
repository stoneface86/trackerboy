
#include "model/BaseTableModel.hpp"


BaseTableModel::BaseTableModel(trackerboy::BaseTable &table, QObject *parent) :
    mBaseTable(table),
    QAbstractListModel(parent)
{
}

int BaseTableModel::rowCount(const QModelIndex &parent) const {
    (void)parent;
    return mBaseTable.size();
}

QVariant BaseTableModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        auto iter = mBaseTable.begin() + index.row();
        QString str("%1 - %2");
        return QVariant(str.arg(QString::number(iter->index), QString::fromStdString(iter->name)));
    } else if (role == Qt::DecorationRole) {
        return iconData(index);
    } else {
        return QVariant();
    }
}

void BaseTableModel::addItem() {
    int row = mBaseTable.size();
    beginInsertRows(QModelIndex(), row, row);
    insertItemInTable();
    endInsertRows();
}

QString BaseTableModel::name(int index) {
    auto iter = mBaseTable.begin() + index;
    return QString::fromStdString(iter->name);
}

void BaseTableModel::setName(int index, QString name) {
    auto iter = mBaseTable.begin() + index;
    mBaseTable.setName(iter->index, name.toStdString());
    emit dataChanged(createIndex(index, 0, nullptr), createIndex(index, 0, nullptr), { Qt::DisplayRole });
}
