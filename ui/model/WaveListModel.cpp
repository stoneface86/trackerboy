
#include "model/WaveListModel.hpp"

WaveListModel::WaveListModel(trackerboy::WaveTable &table, QObject *parent) :
    mTable(table),
    QAbstractListModel(parent)
{
}

int WaveListModel::rowCount(const QModelIndex &parent) const {
    (void)parent;
    return mTable.size();
}

QVariant WaveListModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        auto iter = mTable.begin() + index.row();
        QString str("%1 - %2");
        return QVariant(str.arg(QString::number(iter->id), QString::fromStdString(iter->name)));
    } else {
        return QVariant();
    }
}

